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
#include "../SDKCommonDefine/SDK_Export.h"

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
    std::string m_message; ///< 日志消息
    std::string m_timestamp; ///< 时间戳
    std::string m_fileName; ///< 文件名
    int m_lineNumber; ///< 行号

    /// <summary>
    /// 构造函数，初始化默认值
    /// </summary>
    ST_LogMessage()
        : m_lineNumber(0)
    {
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
    std::queue<ST_LogMessage> m_messageQueue; ///< 消息队列
    std::ofstream m_logFile; ///< 日志文件流
    ST_LogConfig m_config; ///< 日志配置
    std::unique_ptr<std::thread> m_writeThread; ///< 写入线程
    bool m_running; ///< 运行标志
    size_t m_currentFileSize; ///< 当前文件大小
};

/// <summary>
/// 日志宏定义，提供便捷的日志记录接口
/// </summary>
#define LOG_DEBUG(format, ...) LogSystem::Instance().WriteLogFormat(EM_LogLevel::Debug, format, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_INFO(format, ...) LogSystem::Instance().WriteLogFormat(EM_LogLevel::Info, format, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_WARN(format, ...) LogSystem::Instance().WriteLogFormat(EM_LogLevel::Warning, format, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) LogSystem::Instance().WriteLogFormat(EM_LogLevel::Error, format, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_FATAL(format, ...) LogSystem::Instance().WriteLogFormat(EM_LogLevel::Fatal, format, __FILE__, __LINE__, ##__VA_ARGS__)
