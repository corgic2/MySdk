/// <summary>
/// 日志系统头文件
/// </summary>
#pragma once
#include <condition_variable>
#include <fstream>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>
#include "../SDKCommonDefine/SDK_Export.h"
#include "MemoryPool.h"

/// <summary>
/// 日志级别枚举
/// </summary>
enum class EM_LogLevel
{
    Debug, ///< 调试信息
    Info, ///< 一般信息
    Warning, ///< 警告信息
    Error, ///< 错误信息
    Fatal ///< 致命错误
};

/// <summary>
/// 日志系统配置结构体
/// </summary>
struct ST_LogConfig
{
    std::string m_logFilePath; ///< 日志文件路径
    EM_LogLevel m_logLevel; ///< 日志级别
    size_t m_maxQueueSize; ///< 最大队列大小
    size_t m_maxFileSize; ///< 最大文件大小(字节)
    bool m_asyncEnabled; ///< 是否启用异步日志

    /// <summary>
    /// 构造函数，初始化默认配置
    /// </summary>
    ST_LogConfig()
        : m_logLevel(EM_LogLevel::Info)
        , m_maxQueueSize(10000)
        , m_maxFileSize(5 * 1024 * 1024) // 5MB
        , m_asyncEnabled(true)
    {
    }
};

/// <summary>
/// 日志消息结构体
/// </summary>
struct ST_LogMessage
{
    EM_LogLevel m_level; ///< 日志级别
    char* m_message; ///< 日志消息
    size_t m_messageLength; ///< 消息长度
    std::string m_timestamp; ///< 时间戳
    std::string m_fileName; ///< 文件名
    int m_lineNumber; ///< 行号

    /// <summary>
    /// 构造函数，初始化默认值
    /// </summary>
    ST_LogMessage()
        : m_level(EM_LogLevel::Info)
        , m_message(nullptr)
        , m_messageLength(0)
        , m_lineNumber(0)
    {
    }

    /// <summary>
    /// 析构函数
    /// </summary>
    ~ST_LogMessage()
    {
        if (m_message)
        {
            StringMemoryPool::Instance().Deallocate(m_message);
            m_message = nullptr;
        }
    }

    /// <summary>
    /// 设置消息内容
    /// </summary>
    /// <param name="message">消息字符串</param>
    void SetMessage(const std::string& message)
    {
        m_messageLength = message.length();
        m_message = StringMemoryPool::Instance().Allocate(m_messageLength + 1);
        std::copy(message.c_str(), message.c_str() + m_messageLength + 1, m_message);
    }
};

/// <summary>
/// 日志缓冲区结构体
/// </summary>
struct ST_LogBuffer {
    std::string m_data;          ///< 缓冲区数据
    size_t m_size;              ///< 当前大小
    static const size_t MAX_SIZE = 8192; ///< 最大大小

    /// <summary>
    /// 构造函数
    /// </summary>
    ST_LogBuffer() : m_size(0) {
        m_data.reserve(MAX_SIZE);
    }

    /// <summary>
    /// 追加数据
    /// </summary>
    /// <param name="data">数据</param>
    /// <returns>是否成功</returns>
    bool Append(const std::string& data) {
        if (m_size + data.size() > MAX_SIZE) {
            return false;
        }
        m_data.append(data);
        m_size += data.size();
        return true;
    }

    /// <summary>
    /// 清空缓冲区
    /// </summary>
    void Clear() {
        m_data.clear();
        m_size = 0;
    }
};

/// <summary>
/// 日志批处理结构体
/// </summary>
struct ST_LogBatch {
    static const size_t MAX_BATCH_SIZE = 1000;  ///< 最大批处理大小
    std::vector<ST_LogMessage> m_messages;      ///< 消息列表
    size_t m_totalSize;                         ///< 总大小（字节）

    /// <summary>
    /// 构造函数
    /// </summary>
    ST_LogBatch() : m_totalSize(0) {
        m_messages.reserve(MAX_BATCH_SIZE);
    }

    /// <summary>
    /// 添加消息到批处理
    /// </summary>
    /// <param name="msg">日志消息</param>
    /// <returns>是否成功</returns>
    bool Add(ST_LogMessage&& msg) {
        if (m_messages.size() >= MAX_BATCH_SIZE) {
            return false;
        }
        m_totalSize += msg.m_messageLength;
        m_messages.push_back(std::move(msg));
        return true;
    }

    /// <summary>
    /// 清空批处理
    /// </summary>
    void Clear() {
        m_messages.clear();
        m_totalSize = 0;
    }
};

/// <summary>
/// 日志系统类，提供异步日志记录功能
/// </summary>
class SDK_API LogSystem
{
public:
    /// <summary>
    /// 获取日志系统单例
    /// </summary>
    /// <returns>日志系统单例引用</returns>
    static LogSystem& Instance();

    /// <summary>
    /// 初始化日志系统
    /// </summary>
    /// <param name="config">日志配置</param>
    void Initialize(const ST_LogConfig& config);

    /// <summary>
    /// 设置日志文件路径
    /// </summary>
    /// <param name="filePath">文件路径</param>
    void SetLogFile(const std::string& filePath);

    /// <summary>
    /// 设置日志级别
    /// </summary>
    /// <param name="level">日志级别</param>
    void SetLogLevel(EM_LogLevel level);

    /// <summary>
    /// 写入日志
    /// </summary>
    /// <param name="level">日志级别</param>
    /// <param name="message">日志消息</param>
    /// <param name="file">文件名</param>
    /// <param name="line">行号</param>
    void WriteLog(EM_LogLevel level, const std::string& message,
                  const char* file = nullptr, int line = 0);

    /// <summary>
    /// 格式化并写入日志
    /// </summary>
    /// <param name="level">日志级别</param>
    /// <param name="format">格式化字符串</param>
    /// <param name="file">文件名</param>
    /// <param name="line">行号</param>
    /// <param name="args">可变参数列表</param>
    template <typename... Args>
    void WriteLogFormat(EM_LogLevel level, const char* format,
                        const char* file, int line, Args... args)
    {
        std::string message = StringFormat(format, args...);
        WriteLog(level, message, file, line);
    }

    /// <summary>
    /// 刷新日志缓冲区
    /// </summary>
    void Flush();

    /// <summary>
    /// 停止日志系统
    /// </summary>
    void Shutdown();

    /// <summary>
    /// 析构函数
    /// </summary>
    ~LogSystem();

private:
    /// <summary>
    /// 构造函数
    /// </summary>
    LogSystem();
    LogSystem(const LogSystem&) = delete;
    LogSystem& operator=(const LogSystem&) = delete;

private:
    /// <summary>
    /// 异步写入线程函数
    /// </summary>
    void AsyncWrite();

    /// <summary>
    /// 获取当前时间戳
    /// </summary>
    /// <returns>格式化的时间戳字符串</returns>
    std::string GetTimestamp() const;

    /// <summary>
    /// 获取日志级别字符串
    /// </summary>
    /// <param name="level">日志级别</param>
    /// <returns>日志级别对应的字符串</returns>
    const char* GetLevelString(EM_LogLevel level) const;

    /// <summary>
    /// 检查并轮换日志文件
    /// </summary>
    void CheckRotateFile();

    /// <summary>
    /// 写入日志到文件
    /// </summary>
    /// <param name="msg">日志消息</param>
    void WriteLogToFile(const ST_LogMessage& msg);

    /// <summary>
    /// 字符串格式化函数
    /// </summary>
    /// <param name="format">格式化字符串</param>
    /// <param name="args">可变参数列表</param>
    /// <returns>格式化后的字符串</returns>
    template <typename... Args>
    static std::string StringFormat(const char* format, Args... args)
    {
        int size = snprintf(nullptr, 0, format, args...);
        if (size <= 0)
        {
            return "";
        }

        std::string result;
        result.resize(size + 1);
        snprintf(&result[0], size + 1, format, args...);
        result.resize(size);

        return result;
    }

private:
    std::mutex m_mutex; ///< 互斥锁
    std::condition_variable m_condition; ///< 条件变量
    std::queue<ST_LogBatch> m_messageQueue; ///< 消息队列
    std::ofstream m_logFile; ///< 日志文件流
    ST_LogConfig m_config; ///< 日志配置
    std::unique_ptr<std::thread> m_writeThread; ///< 写入线程
    bool m_running; ///< 运行标志
    size_t m_currentFileSize; ///< 当前文件大小
    std::chrono::steady_clock::time_point m_lastFlushTime; ///< 上次刷新时间
    static constexpr size_t BUFFER_SIZE = 8192; ///< 缓冲区大小
    static constexpr auto FLUSH_INTERVAL = std::chrono::seconds(1); ///< 刷新间隔
    ST_LogBuffer m_currentBuffer; ///< 当前缓冲区
    ST_LogBuffer m_nextBuffer; ///< 下一个缓冲区
    std::vector<std::unique_ptr<ST_LogBuffer>> m_bufferPool; ///< 缓冲区池
    std::mutex m_bufferMutex; ///< 缓冲区互斥锁
    static constexpr size_t BUFFER_POOL_SIZE = 4; ///< 缓冲区池大小
    ST_LogBatch m_currentBatch; ///< 当前批处理
    std::mutex m_batchMutex; ///< 批处理互斥锁
    static constexpr size_t MAX_BATCH_WAIT_MS = 100; ///< 最大批处理等待时间（毫秒）
};

/// <summary>
/// 日志宏定义，提供便捷的日志记录接口
/// </summary>
#define LOG_DEBUG(format, ...) LogSystem::Instance().WriteLogFormat(EM_LogLevel::Debug, format, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_INFO(format, ...) LogSystem::Instance().WriteLogFormat(EM_LogLevel::Info, format, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_WARN(format, ...) LogSystem::Instance().WriteLogFormat(EM_LogLevel::Warning, format, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) LogSystem::Instance().WriteLogFormat(EM_LogLevel::Error, format, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_FATAL(format, ...) LogSystem::Instance().WriteLogFormat(EM_LogLevel::Fatal, format, __FILE__, __LINE__, ##__VA_ARGS__)
