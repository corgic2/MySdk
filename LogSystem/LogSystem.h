#pragma once
#include <cerrno>
#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
#include <mutex>
#include <source_location>
#include <string_view>
#include "../ThreadPool/ThreadPool.h"

class LogSystemSyncThreadPool
{
public:
    LogSystemSyncThreadPool();
    ~LogSystemSyncThreadPool();

private:
    ThreadPool m_threadPool;
};
class LogSystem
{
private:
    LogSystem();
    ~LogSystem();

    LogSystem& operator=(const LogSystem& obj) = delete;
    LogSystem(const LogSystem& obj) = delete;

public:
    enum EM_LogLevel
    {
        INFO    = 0,
        DEBUG   = 1,
        WARNING = 2,
        ERROR   = 3,
    };

public:
    // 单例访问
    static LogSystem& instance() noexcept;

    // 核心日志接口
    template <typename... Args>
    void Log(EM_LogLevel level, const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args)
    {
        if (level < m_level.load(std::memory_order_relaxed))
        {
            return;
        }

        // 格式化时间戳
        auto now = std::chrono::system_clock::now();
        std::string timestamp = std::format("{:%Y-%m-%d %H:%M:%S}", now);

        // 组合完整日志消息
#if DEBUG_LOG 
        std::string message = std::format("[{}] [{}:{}] [{}] {}\n", timestamp, loc.file_name(), loc.line(), LevelToString(level), std::vformat(fmt.get(), std::make_format_args<std::format_context>(args...)));
#else
        std::string message = std::format("[{}] [{}] {}\n", timestamp, LevelToString(level), std::vformat(fmt.get(), std::make_format_args<std::format_context>(args...)));
#endif
        Write(message);
    }

    // 配置接口
    void SetLevel(EM_LogLevel level) noexcept
    {
        m_level.store(level, std::memory_order_relaxed);
    }

    void SetFile(std::string filename);

private:
    // 实现细节
    void Write(std::string_view message);
    std::string LevelToString(EM_LogLevel level);
    std::ofstream m_file;
    std::mutex m_mutex;
    std::atomic<EM_LogLevel> m_level{EM_LogLevel::INFO};
    std::string m_filePath;
};


#define LOG_INFO(fmt, ...) \
    do { \
            LogSystem::instance().log(LogSystem::EM_LogLevel::INFO,std::source_location::current(), fmt, ##__VA_ARGS__); \
    } while(0)

#define LOG_DEBUG(fmt, ...) \
    do { \
            LogSystem::instance().log(LogSystem::EM_LogLevel::DEBUG, std::source_location::current(), fmt, ##__VA_ARGS__); \
    } while(0)

#define LOG_WARNING(fmt, ...) \
    do { \
            LogSystem::instance().log(LogSystem::EM_LogLevel::WARNING, std::source_location::current(), fmt, ##__VA_ARGS__); \
    } while(0)

#define LOG_ERROR(fmt, ...) \
    do { \
            LogSystem::instance().log(LogSystem::EM_LogLevel::ERROR, std::source_location::current(), fmt, ##__VA_ARGS__); \
    } while(0)
