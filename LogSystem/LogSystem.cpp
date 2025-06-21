#include "LogSystem.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QTextCodec>

// LogWriteThread 实现
LogWriteThread::LogWriteThread(QObject* parent)
    : QThread(parent), m_currentFileSize(0), m_running(0), m_flushTimer(nullptr)
{
}

LogWriteThread::~LogWriteThread()
{
    Stop();
}

void LogWriteThread::SetConfig(const ST_LogConfig& config)
{
    QMutexLocker locker(&m_queueMutex);
    m_config = config;
}

void LogWriteThread::AddMessage(const ST_LogMessage& message)
{
    QMutexLocker locker(&m_queueMutex);

    if (m_messageQueue.size() >= m_config.m_maxQueueSize)
    {
        return; // 队列已满，丢弃消息
    }

    m_messageQueue.enqueue(message);
    m_condition.wakeOne();
}

void LogWriteThread::Stop()
{
    m_running.store(0);
    m_condition.wakeAll();

    if (isRunning())
    {
        wait(5000); // 等待5秒
        if (isRunning())
        {
            terminate(); // 强制终止
            wait(1000);
        }
    }

    QMutexLocker fileLock(&m_fileMutex);
    if (m_logFile.isOpen())
    {
        m_textStream.flush();
        m_logFile.close();
    }

    if (m_flushTimer)
    {
        m_flushTimer->stop();
        delete m_flushTimer;
        m_flushTimer = nullptr;
    }
}

void LogWriteThread::Flush()
{
    QMutexLocker fileLock(&m_fileMutex);
    if (m_textStream.device())
    {
        m_textStream.flush();
    }
}

void LogWriteThread::run()
{
    m_running.store(1);

    // 创建定时器用于定期刷新
    m_flushTimer = new QTimer();
    m_flushTimer->setInterval(m_config.m_flushInterval);
    connect(m_flushTimer, &QTimer::timeout, this, &LogWriteThread::OnFlushTimer);
    m_flushTimer->start();

    // 初始化日志文件
    InitializeLogFile();

    while (m_running.load() == 1)
    {
        ST_LogMessage message;
        bool hasMessage = false;
        
        {
            QMutexLocker locker(&m_queueMutex);
            if (m_messageQueue.isEmpty())
            {
                m_condition.wait(&m_queueMutex, 100); // 等待100ms
            }

            if (!m_messageQueue.isEmpty())
            {
                message = m_messageQueue.dequeue();
                hasMessage = true;
            }
        }

        if (hasMessage)
        {
            WriteLogToFile(message);
        }
    }

    // 处理剩余消息
    QMutexLocker locker(&m_queueMutex);
    while (!m_messageQueue.isEmpty())
    {
        ST_LogMessage message = m_messageQueue.dequeue();
        WriteLogToFile(message);
    }

    // 最终刷新
    Flush();

    if (m_flushTimer)
    {
        m_flushTimer->stop();
        delete m_flushTimer;
        m_flushTimer = nullptr;
    }
}

void LogWriteThread::OnFlushTimer()
{
    Flush();
}

void LogWriteThread::WriteLogToFile(const ST_LogMessage& message)
{
    QMutexLocker fileLock(&m_fileMutex);

    if (m_config.m_logFilePath.isEmpty() || !m_textStream.device())
    {
        return;
    }
    
    CheckRotateFile();

    // 构建日志行
    QString logLine = QString("[%1] [%2] ").arg(message.m_timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz")).arg(GetLevelString(message.m_level));

    if (!message.m_fileName.isEmpty())
    {
        logLine += QString("(%1:%2) ").arg(message.m_fileName).arg(message.m_lineNumber);
    }

    logLine += QString("- %1\n").arg(message.m_message);

    // 写入缓冲区
    {
        QMutexLocker bufferLock(&m_bufferMutex);
        m_currentLogBuffer += logLine;

        // 如果缓冲区达到一定大小，立即写入文件
        if (m_currentLogBuffer.length() > 1024) // 1KB缓冲
        {
            m_textStream << m_currentLogBuffer;
            m_textStream.flush();
            m_currentFileSize.fetchAndAddOrdered(m_currentLogBuffer.toUtf8().size());
            m_currentLogBuffer.clear();
        }
    }
}

void LogWriteThread::CheckRotateFile()
{
    if (m_currentFileSize.load() >= m_config.m_maxFileSize)
    {
        // 先刷新缓冲区
        {
            QMutexLocker bufferLock(&m_bufferMutex);
            if (!m_currentLogBuffer.isEmpty())
            {
                m_textStream << m_currentLogBuffer;
                m_textStream.flush();
                m_currentLogBuffer.clear();
            }
        }
        
        m_logFile.close();
        
        // 重命名当前日志文件
        QString newFileName = m_config.m_logFilePath + "." + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");

        QFile::rename(m_config.m_logFilePath, newFileName);

        // 重新初始化日志文件
        InitializeLogFile();
    }
}

QString LogWriteThread::GetLevelString(EM_LogLevel level) const
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

void LogWriteThread::InitializeLogFile()
{
    if (m_config.m_logFilePath.isEmpty())
    {
        return;
    }
    
    // 确保目录存在
    EnsureDirectoryExists(m_config.m_logFilePath);
    
    // 检查文件是否存在
    bool fileExists = QFile::exists(m_config.m_logFilePath);
    
    m_logFile.setFileName(m_config.m_logFilePath);
    
    if (!m_logFile.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        qDebug() << "Failed to open log file:" << m_config.m_logFilePath;
        return;
    }
    
    m_textStream.setDevice(&m_logFile);
    // 强制设置UTF-8编码，确保中文字符正确显示
    m_textStream.setCodec("UTF-8");
    // 禁用自动检测，强制使用UTF-8
    m_textStream.setAutoDetectUnicode(false);
    // 设置生成Unicode字节序标记
    m_textStream.setGenerateByteOrderMark(false); // 我们手动写入BOM
    
    // 如果是新文件，写入UTF-8 BOM
    if (!fileExists)
    {
        WriteUtf8Bom();
    }
    
    m_currentFileSize.store(m_logFile.size());
}

void LogWriteThread::WriteUtf8Bom()
{
    // 写入UTF-8 BOM (EF BB BF)
    const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
    m_logFile.write(reinterpret_cast<const char*>(bom), 3);
    m_logFile.flush();
}

void LogWriteThread::EnsureDirectoryExists(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists())
    {
        dir.mkpath(".");
    }
}

// LogSystem 实现
LogSystem::LogSystem(QObject* parent)
    : QObject(parent), m_writeThread(nullptr), m_initialized(0)
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
    QMutexLocker locker(&m_mutex);

    if (m_initialized.load() == 1)
    {
        return;
    }

    m_config = config;

    if (m_config.m_asyncEnabled)
    {
        m_writeThread = new LogWriteThread(this);
        m_writeThread->SetConfig(m_config);
        m_writeThread->start();
    }

    m_initialized.store(1);
}

void LogSystem::SetLogFile(const QString& filePath)
{
    QMutexLocker locker(&m_mutex);
    m_config.m_logFilePath = filePath;

    if (m_writeThread)
    {
        ST_LogConfig newConfig = m_config;
        newConfig.m_logFilePath = filePath;
        m_writeThread->SetConfig(newConfig);
    }
}

void LogSystem::SetLogLevel(EM_LogLevel level)
{
    QMutexLocker locker(&m_mutex);
    m_config.m_logLevel = level;

    if (m_writeThread)
    {
        ST_LogConfig newConfig = m_config;
        newConfig.m_logLevel = level;
        m_writeThread->SetConfig(newConfig);
    }
}

void LogSystem::WriteLog(EM_LogLevel level, const QString& message, const char* file, int line)
{
    if (level < m_config.m_logLevel)
    {
        return;
    }
    
    ST_LogMessage logMsg;
    logMsg.m_level = level;
    logMsg.m_message = message;
    logMsg.m_timestamp = QDateTime::currentDateTime();
    logMsg.m_fileName = file ? GetBaseName(file) : QString();
    logMsg.m_lineNumber = line;
    
    if (m_config.m_asyncEnabled && m_writeThread)
    {
        m_writeThread->AddMessage(logMsg);
    }
    else
    {
        // 同步写入到控制台作为备用
        QString logLine = QString("[%1] [%2] ").arg(logMsg.m_timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz")).arg(GetLevelString(logMsg.m_level));
        
        if (!logMsg.m_fileName.isEmpty())
        {
            logLine += QString("(%1:%2) ").arg(logMsg.m_fileName).arg(logMsg.m_lineNumber);
        }
        
        logLine += QString("- %1").arg(logMsg.m_message);
        
        qDebug() << logLine;
    }
}

void LogSystem::WriteLog(EM_LogLevel level, const std::string& message, const char* file, int line)
{
    // 将std::string转换为QString，确保正确处理UTF-8编码
    WriteLog(level, QString::fromUtf8(message.c_str()), file, line);
}

void LogSystem::Flush()
{
    if (m_writeThread)
    {
        m_writeThread->Flush();
    }
}

void LogSystem::Shutdown()
{
    QMutexLocker locker(&m_mutex);

    if (m_initialized.load() == 0)
    {
        return;
    }

    if (m_writeThread)
    {
        m_writeThread->Stop();
        delete m_writeThread;
        m_writeThread = nullptr;
    }

    m_initialized.store(0);
}

QString LogSystem::GetBaseName(const char* filePath) const
{
    if (!filePath)
    {
        return QString();
    }

    QFileInfo fileInfo(QString::fromLocal8Bit(filePath));
    return fileInfo.baseName();
}

QString LogSystem::GetLevelString(EM_LogLevel level) const
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
