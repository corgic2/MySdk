#include "LogSystem.h"

// 单例初始化
LogSystem& LogSystem::instance() noexcept
{
    static LogSystem instance;
    return instance;
}

// 构造函数
LogSystem::LogSystem()
{
}

LogSystem::~LogSystem()
{
    m_file.close();
}

void LogSystem::set_file(std::string filename)
{
    m_filePath = filename;
    m_file.open(m_filePath, std::ios::app);
}

// 文件写入（带异常处理）
void LogSystem::write(std::string_view message)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    try
    {
        m_file << message;
        m_file.flush(); // 根据配置决定刷新频率
    }
    catch (const std::exception& e)
    {
        // 处理写入失败（如切换备用存储）
        std::cerr << "Log write failed: " << e.what() << std::endl;
    }
}

std::string LogSystem::LevelToString(EM_LogLevel level)
{
    switch (level)
    {
        case EM_LogLevel::INFO:
            return "INFO";
        case EM_LogLevel::DEBUG:
            return "DEBUG";
        case EM_LogLevel::WARNING:
            return "WARNING";
        case EM_LogLevel::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}
