/// <summary>
/// 基于QtCore的日志系统头文件
/// </summary>
#pragma once
#include <string>
#include <QtCore/QAtomicInt>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QObject>
#include <QtCore/QQueue>
#include <QtCore/QString>
#include <QtCore/QTextStream>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QWaitCondition>
#include "../SDKCommonDefine/SDK_Export.h"

/// <summary>
/// 日志级别枚举
/// </summary>
enum class EM_LogLevel
{
    Debug,
    ///< 调试信息
    Info,
    ///< 一般信息
    Warning,
    ///< 警告信息
    Error,
    ///< 错误信息
    Fatal ///< 致命错误
};

/// <summary>
/// 日志系统配置结构体
/// </summary>
struct ST_LogConfig
{
    QString m_logFilePath;  ///< 日志文件路径
    EM_LogLevel m_logLevel; ///< 日志级别
    qint64 m_maxFileSize;   ///< 最大文件大小(字节)
    bool m_asyncEnabled;    ///< 是否启用异步日志
    int m_maxQueueSize;     ///< 最大队列大小
    int m_flushInterval;    ///< 刷新间隔(毫秒)

    /// <summary>
    /// 构造函数，初始化默认配置
    /// </summary>
    ST_LogConfig()
        : m_logLevel(EM_LogLevel::Info), m_maxFileSize(5 * 1024 * 1024)      // 5MB
        , m_asyncEnabled(true), m_maxQueueSize(10000), m_flushInterval(1000) // 1秒
    {
    }
};

/// <summary>
/// 日志消息结构体
/// </summary>
struct ST_LogMessage
{
    EM_LogLevel m_level;   ///< 日志级别
    QString m_message;     ///< 日志消息
    QDateTime m_timestamp; ///< 时间戳
    QString m_fileName;    ///< 文件名
    int m_lineNumber;      ///< 行号

    /// <summary>
    /// 构造函数，初始化默认值
    /// </summary>
    ST_LogMessage()
        : m_level(EM_LogLevel::Info), m_timestamp(QDateTime::currentDateTime()), m_lineNumber(0)
    {
    }

    /// <summary>
    /// 构造函数
    /// </summary>
    /// <param name="level">日志级别</param>
    /// <param name="message">消息内容</param>
    /// <param name="fileName">文件名</param>
    /// <param name="lineNumber">行号</param>
    ST_LogMessage(EM_LogLevel level, const QString& message, const QString& fileName = QString(), int lineNumber = 0)
        : m_level(level), m_message(message), m_timestamp(QDateTime::currentDateTime()), m_fileName(fileName), m_lineNumber(lineNumber)
    {
    }
};

/// <summary>
/// 日志写入线程类
/// </summary>
class SDK_API LogWriteThread : public QThread
{
    Q_OBJECT;

public:
    /// <summary>
    /// 构造函数
    /// </summary>
    /// <param name="parent">父对象</param>
    explicit LogWriteThread(QObject* parent = nullptr);

    /// <summary>
    /// 析构函数
    /// </summary>
    ~LogWriteThread() override;

    /// <summary>
    /// 设置日志配置
    /// </summary>
    /// <param name="config">日志配置</param>
    void SetConfig(const ST_LogConfig& config);

    /// <summary>
    /// 添加日志消息到队列
    /// </summary>
    /// <param name="message">日志消息</param>
    void AddMessage(const ST_LogMessage& message);

    /// <summary>
    /// 停止线程
    /// </summary>
    void Stop();

    /// <summary>
    /// 刷新缓冲区
    /// </summary>
    void Flush();

protected:
    /// <summary>
    /// 线程运行函数
    /// </summary>
    void run() override;

private slots:
    /// <summary>
    /// 定时刷新槽函数
    /// </summary>
    void OnFlushTimer();

private:
    /// <summary>
    /// 写入日志到文件
    /// </summary>
    /// <param name="message">日志消息</param>
    void WriteLogToFile(const ST_LogMessage& message);

    /// <summary>
    /// 检查并轮换日志文件
    /// </summary>
    void CheckRotateFile();

    /// <summary>
    /// 获取日志级别字符串
    /// </summary>
    /// <param name="level">日志级别</param>
    /// <returns>级别字符串</returns>
    QString GetLevelString(EM_LogLevel level) const;

    /// <summary>
    /// 初始化日志文件
    /// </summary>
    void InitializeLogFile();

    /// <summary>
    /// 写入UTF-8 BOM
    /// </summary>
    void WriteUtf8Bom();

    /// <summary>
    /// 确保目录存在
    /// </summary>
    /// <param name="filePath">文件路径</param>
    void EnsureDirectoryExists(const QString& filePath);

private:
    QQueue<ST_LogMessage> m_messageQueue; ///< 消息队列
    QMutex m_queueMutex;                  ///< 队列互斥锁
    QWaitCondition m_condition;           ///< 条件变量
    QMutex m_fileMutex;                   ///< 文件互斥锁
    QFile m_logFile;                      ///< 日志文件
    QTextStream m_textStream;             ///< 文本流
    ST_LogConfig m_config;                ///< 日志配置
    QAtomicInt m_currentFileSize;         ///< 当前文件大小
    QAtomicInt m_running;                 ///< 运行标志
    QTimer* m_flushTimer;                 ///< 刷新定时器
    QString m_currentLogBuffer;           ///< 当前日志缓冲区
    QMutex m_bufferMutex;                 ///< 缓冲区互斥锁
};

/// <summary>
/// 日志系统类，提供异步日志记录功能
/// </summary>
class SDK_API LogSystem : public QObject
{
    Q_OBJECT;

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
    void SetLogFile(const QString& filePath);

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
    void WriteLog(EM_LogLevel level, const QString& message, const char* file = nullptr, int line = 0);

    /// <summary>
    /// 写入日志 (std::string版本，用于兼容)
    /// </summary>
    /// <param name="level">日志级别</param>
    /// <param name="message">日志消息</param>
    /// <param name="file">文件名</param>
    /// <param name="line">行号</param>
    void WriteLog(EM_LogLevel level, const std::string& message, const char* file = nullptr, int line = 0);

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
    ~LogSystem() override;

private:
    /// <summary>
    /// 构造函数
    /// </summary>
    explicit LogSystem(QObject* parent = nullptr);
    LogSystem(const LogSystem&) = delete;
    LogSystem& operator=(const LogSystem&) = delete;

    /// <summary>
    /// 获取文件基名
    /// </summary>
    /// <param name="filePath">文件路径</param>
    /// <returns>文件基名</returns>
    QString GetBaseName(const char* filePath) const;

    /// <summary>
    /// 获取日志级别字符串
    /// </summary>
    /// <param name="level">日志级别</param>
    /// <returns>级别字符串</returns>
    QString GetLevelString(EM_LogLevel level) const;

private:
    ST_LogConfig m_config;         ///< 日志配置
    LogWriteThread* m_writeThread; ///< 写入线程
    QMutex m_mutex;                ///< 互斥锁
    QAtomicInt m_initialized;      ///< 初始化标志
};

/// <summary>
/// 日志宏定义，提供便捷的日志记录接口
/// </summary>
#define LOG_DEBUG(message) LogSystem::Instance().WriteLog(EM_LogLevel::Debug, std::string(message), __FILE__, __LINE__)
#define LOG_INFO(message) LogSystem::Instance().WriteLog(EM_LogLevel::Info, std::string(message), __FILE__, __LINE__)
#define LOG_WARN(message) LogSystem::Instance().WriteLog(EM_LogLevel::Warning, std::string(message), __FILE__, __LINE__)
#define LOG_ERROR(message) LogSystem::Instance().WriteLog(EM_LogLevel::Error, std::string(message), __FILE__, __LINE__)
#define LOG_FATAL(message) LogSystem::Instance().WriteLog(EM_LogLevel::Fatal, std::string(message), __FILE__, __LINE__)
