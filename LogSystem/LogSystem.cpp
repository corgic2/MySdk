#include "LogSystem.h"
#include <chrono>
#include <ctime>
#include <direct.h>
#include <io.h>
#include <iomanip>
#include <sstream>

LogSystem::LogSystem()
    : m_running(false)
    , m_currentFileSize(0)
    , m_lastFlushTime(std::chrono::steady_clock::now())
{
    // 初始化缓冲区池
    for (size_t i = 0; i < BUFFER_POOL_SIZE; ++i)
    {
        m_bufferPool.push_back(std::make_unique<ST_LogBuffer>());
    }
}

LogSystem::~LogSystem()
{
    Shutdown();
}

LogSystem& LogSystem::Instance()
{
    static LogSystem instance;
    return instance;
}

void LogSystem::Initialize(const ST_LogConfig& config)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config = config;

    if (!m_config.m_logFilePath.empty())
    {
        m_logFile.open(m_config.m_logFilePath, std::ios::app);
        if (m_logFile.is_open())
        {
            struct _stat64 fileStat;
            if (_stat64(m_config.m_logFilePath.c_str(), &fileStat) == 0)
            {
                m_currentFileSize = fileStat.st_size;
            }
        }
    }

    if (m_config.m_asyncEnabled && !m_running)
    {
        m_running = true;
        m_writeThread = std::make_unique<std::thread>(&LogSystem::AsyncWrite, this);
    }
}

void LogSystem::SetLogFile(const std::string& filePath)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_logFile.is_open())
    {
        m_logFile.close();
    }

    m_config.m_logFilePath = filePath;
    m_logFile.open(filePath, std::ios::app);
    if (m_logFile.is_open())
    {
        struct _stat64 fileStat;
        if (_stat64(filePath.c_str(), &fileStat) == 0)
        {
            m_currentFileSize = fileStat.st_size;
        }
    }
}

void LogSystem::SetLogLevel(EM_LogLevel level)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config.m_logLevel = level;
}

void LogSystem::WriteLog(EM_LogLevel level, const std::string& message,
                         const char* file, int line)
{
    if (level < m_config.m_logLevel)
    {
        return;
    }

    ST_LogMessage logMsg;
    logMsg.m_level = level;
    logMsg.SetMessage(message);
    logMsg.m_timestamp = GetTimestamp();
    logMsg.m_fileName = file ? file : "";
    logMsg.m_lineNumber = line;

    if (m_config.m_asyncEnabled)
    {
        bool needFlush = false;
        {
            std::lock_guard<std::mutex> lock(m_batchMutex);
            if (!m_currentBatch.Add(std::move(logMsg)))
            {
                // 当前批次已满，通知异步线程处理
                m_messageQueue.push(std::move(m_currentBatch));
                m_currentBatch.Clear();
                m_currentBatch.Add(std::move(logMsg));
                needFlush = true;
            }
        }
        if (needFlush)
        {
            m_condition.notify_one();
        }
    }
    else
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        WriteLogToFile(logMsg);
    }
}

void LogSystem::AsyncWrite()
{
    ST_LogBatch localBatch;
    std::chrono::steady_clock::time_point lastProcessTime = std::chrono::steady_clock::now();

    while (m_running)
    {
        bool hasBatch = false;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            auto now = std::chrono::steady_clock::now();
            auto timeSinceLastProcess = std::chrono::duration_cast<std::chrono::milliseconds>(
                                                                                              now - lastProcessTime).count();

            // 等待新的批次或超时
            m_condition.wait_for(lock, std::chrono::milliseconds(MAX_BATCH_WAIT_MS - timeSinceLastProcess),
                                 [this, timeSinceLastProcess]()
                                 {
                                     return !m_messageQueue.empty() || !m_running || timeSinceLastProcess >= MAX_BATCH_WAIT_MS;
                                 });

            if (!m_running && m_messageQueue.empty())
            {
                break;
            }

            // 获取待处理的批次
            if (!m_messageQueue.empty())
            {
                localBatch = std::move(m_messageQueue.front());
                m_messageQueue.pop();
                hasBatch = true;
            }
            else if (timeSinceLastProcess >= MAX_BATCH_WAIT_MS)
            {
                // 超时，处理当前批次
                std::lock_guard<std::mutex> batchLock(m_batchMutex);
                if (!m_currentBatch.m_messages.empty())
                {
                    localBatch = std::move(m_currentBatch);
                    m_currentBatch.Clear();
                    hasBatch = true;
                }
            }
        }

        // 处理批次
        if (hasBatch)
        {
            for (auto& msg : localBatch.m_messages)
            {
                WriteLogToFile(msg);
            }
            localBatch.Clear();
            lastProcessTime = std::chrono::steady_clock::now();
        }

        // 定期检查并清理缓冲区池
        static size_t cleanupCounter = 0;
        if (++cleanupCounter >= 1000)
        {
            cleanupCounter = 0;
            std::lock_guard<std::mutex> lock(m_bufferMutex);
            while (m_bufferPool.size() > BUFFER_POOL_SIZE)
            {
                m_bufferPool.pop_back();
            }
        }
    }

    // 确保所有数据都被写入
    Flush();
}

void LogSystem::WriteLogToFile(const ST_LogMessage& msg)
{
    if (!m_logFile.is_open())
    {
        return;
    }

    CheckRotateFile();

    // 预分配字符串缓冲区，避免频繁的内存分配
    thread_local std::string logLine;
    logLine.clear();
    logLine.reserve(1024); // 预分配1KB空间

    // 使用string的append代替stringstream，减少内存分配
    logLine.append("[");
    logLine.append(msg.m_timestamp);
    logLine.append("] [");
    logLine.append(GetLevelString(msg.m_level));
    logLine.append("] ");

    if (!msg.m_fileName.empty())
    {
        logLine.append("(");
        logLine.append(msg.m_fileName);
        logLine.append(":");
        logLine.append(std::to_string(msg.m_lineNumber));
        logLine.append(") ");
    }

    logLine.append("- ");
    if (msg.m_message)
    {
        logLine.append(msg.m_message, msg.m_messageLength);
    }
    logLine.append("\n");
    
    bool needFlush = false;
    {
        std::lock_guard<std::mutex> lock(m_bufferMutex);
        if (!m_currentBuffer.Append(logLine))
        {
            // 当前缓冲区已满，压缩并写入文件
            if (m_currentBuffer.m_size > 0)
            {
                ST_CompressedLogBlock block;
                block.m_originalSize = m_currentBuffer.m_size;
                block.m_timestamp = std::time(nullptr);
                block.m_data = LogCompressor::Compress(m_currentBuffer.m_data, m_config.m_compressLevel);
                block.WriteToFile(m_logFile);
            }

            // 切换到下一个缓冲区
            if (!m_bufferPool.empty())
            {
                auto buffer = std::move(m_bufferPool.back());
                m_bufferPool.pop_back();
                std::swap(m_currentBuffer, *buffer);
                m_bufferPool.push_back(std::move(buffer));
            }
            m_currentBuffer.Clear();
            m_currentBuffer.Append(logLine);
            needFlush = true;
        }
    }

    if (needFlush || 
        std::chrono::steady_clock::now() - m_lastFlushTime >= FLUSH_INTERVAL)
    {
        Flush();
    }

    m_currentFileSize += logLine.length();
}

void LogSystem::CheckRotateFile()
{
    if (m_currentFileSize >= m_config.m_maxFileSize)
    {
        m_logFile.close();

        // 重命名当前日志文件
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");

        std::string newFileName = m_config.m_logFilePath + "." + ss.str();
        rename(m_config.m_logFilePath.c_str(), newFileName.c_str());

        // 创建新的日志文件
        m_logFile.open(m_config.m_logFilePath, std::ios::app);
        m_currentFileSize = 0;
    }
}

std::string LogSystem::GetTimestamp() const
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

const char* LogSystem::GetLevelString(EM_LogLevel level) const
{
    switch (level)
    {
        case EM_LogLevel::Debug:
            return "DEBUG";
        case EM_LogLevel::Info:
            return "INFO";
        case EM_LogLevel::Warning:
            return "WARN";
        case EM_LogLevel::Error:
            return "ERROR";
        case EM_LogLevel::Fatal:
            return "FATAL";
        default:
            return "UNKNOWN";
    }
}

void LogSystem::Flush()
{
    if (!m_logFile.is_open())
    {
        return;
    }

    ST_LogBuffer bufferToWrite;
    {
        std::lock_guard<std::mutex> lock(m_bufferMutex);
        std::swap(bufferToWrite, m_currentBuffer);
    }

    if (bufferToWrite.m_size > 0)
    {
        // 压缩并写入文件
        ST_CompressedLogBlock block;
        block.m_originalSize = bufferToWrite.m_size;
        block.m_timestamp = std::time(nullptr);
        block.m_data = LogCompressor::Compress(bufferToWrite.m_data, m_config.m_compressLevel);
        block.WriteToFile(m_logFile);
        m_logFile.flush();
        m_lastFlushTime = std::chrono::steady_clock::now();
    }
}

void LogSystem::Shutdown()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_running)
        {
            return;
        }
        m_running = false;
    }

    m_condition.notify_all();
    if (m_writeThread && m_writeThread->joinable())
    {
        m_writeThread->join();
    }

    Flush(); // 确保所有数据都被写入

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_logFile.is_open())
    {
        m_logFile.close();
    }
}
