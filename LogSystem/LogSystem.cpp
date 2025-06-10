#include "LogSystem.h"
#include <chrono>
#include <ctime>
#include <direct.h>
#include <io.h>
#include <iomanip>
#include <sstream>

LogSystem::LogSystem()
    : m_running(false),
      m_currentFileSize(0)
{
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
    logMsg.m_message = message;
    logMsg.m_timestamp = GetTimestamp();
    logMsg.m_fileName = file ? file : "";
    logMsg.m_lineNumber = line;

    if (m_config.m_asyncEnabled)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_messageQueue.size() < m_config.m_maxQueueSize)
        {
            m_messageQueue.push(std::move(logMsg));
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
    while (m_running)
    {
        ST_LogMessage msg;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condition.wait(lock, [this]()
            {
                return !m_messageQueue.empty() || !m_running;
            });

            if (!m_running && m_messageQueue.empty())
            {
                break;
            }

            if (!m_messageQueue.empty())
            {
                msg = std::move(m_messageQueue.front());
                m_messageQueue.pop();
            }
        }

        if (!msg.m_message.empty())
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            WriteLogToFile(msg);
        }
    }
}

void LogSystem::WriteLogToFile(const ST_LogMessage& msg)
{
    if (!m_logFile.is_open())
    {
        return;
    }

    CheckRotateFile();

    std::stringstream ss;
    ss << "[" << msg.m_timestamp << "] "
        << "[" << GetLevelString(msg.m_level) << "] ";

    if (!msg.m_fileName.empty())
    {
        ss << "(" << msg.m_fileName << ":" << msg.m_lineNumber << ") ";
    }

    ss << "- " << msg.m_message << "\n";

    std::string logLine = ss.str();
    m_logFile.write(logLine.c_str(), logLine.length());
    m_logFile.flush();
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
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_logFile.is_open())
    {
        m_logFile.flush();
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

    std::lock_guard<std::mutex> lock(m_mutex);
    while (!m_messageQueue.empty())
    {
        WriteLogToFile(m_messageQueue.front());
        m_messageQueue.pop();
    }

    if (m_logFile.is_open())
    {
        m_logFile.close();
    }
}
