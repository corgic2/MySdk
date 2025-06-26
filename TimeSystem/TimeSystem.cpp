/// <summary>
/// 时间系统类实现文件 - 基于C++17时间库的计时功能实现
/// </summary>
#include "TimeSystem.h"
#include <iomanip>
#include <iostream>
#include <sstream>

/// <summary>
/// 获取时间系统单例
/// </summary>
/// <returns>时间系统单例引用</returns>
TimeSystem& TimeSystem::Instance()
{
    static TimeSystem instance;
    return instance;
}

/// <summary>
/// 构造函数
/// </summary>
TimeSystem::TimeSystem()
{
}

/// <summary>
/// 析构函数
/// </summary>
TimeSystem::~TimeSystem()
{
    ClearAllTimings();
}

/// <summary>
/// 开始计时
/// </summary>
/// <param name="taskName">任务名称，用于标识不同的计时任务</param>
/// <returns>如果成功开始计时返回true，否则返回false</returns>
bool TimeSystem::StartTiming(const std::string& taskName)
{
    if (taskName.empty())
    {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    // 检查任务是否已经存在
    auto it = m_timingTasks.find(taskName);
    if (it != m_timingTasks.end() && it->second.m_isRunning)
    {
        return false; // 任务已经在运行
    }

    // 创建新的计时任务
    m_timingTasks[taskName] = ST_TimingInfo(taskName);
    return true;
}

/// <summary>
/// 终止计时并返回耗时
/// </summary>
/// <param name="taskName">任务名称</param>
/// <param name="unit">返回的时间单位</param>
/// <returns>任务耗时，如果任务不存在返回-1</returns>
double TimeSystem::StopTiming(const std::string& taskName, EM_TimeUnit unit)
{
    if (taskName.empty())
    {
        return -1.0;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_timingTasks.find(taskName);
    if (it == m_timingTasks.end() || !it->second.m_isRunning)
    {
        return -1.0; // 任务不存在或未运行
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double elapsedTime = CalculateDuration(it->second.m_startTime, endTime, unit);

    // 移除任务
    m_timingTasks.erase(it);

    return elapsedTime;
}

/// <summary>
/// 终止计时并通过日志系统输出耗时信息
/// </summary>
/// <param name="taskName">任务名称</param>
/// <param name="logLevel">日志级别</param>
/// <param name="unit">时间单位</param>
/// <param name="customMessage">自定义消息前缀</param>
/// <returns>任务耗时，如果任务不存在返回-1</returns>
double TimeSystem::StopTimingWithLog(const std::string& taskName, EM_TimingLogLevel logLevel, EM_TimeUnit unit, const std::string& customMessage)
{
    double elapsedTime = StopTiming(taskName, unit);

    if (elapsedTime >= 0.0)
    {
        std::string logMessage = FormatTimingLogMessage(taskName, elapsedTime, unit, customMessage);
        WriteToLogSystem(logLevel, logMessage);
    }

    return elapsedTime;
}

/// <summary>
/// 获取正在进行的任务耗时（不停止计时）
/// </summary>
/// <param name="taskName">任务名称</param>
/// <param name="unit">时间单位</param>
/// <returns>当前耗时，如果任务不存在返回-1</returns>
double TimeSystem::GetElapsedTime(const std::string& taskName, EM_TimeUnit unit) const
{
    if (taskName.empty())
    {
        return -1.0;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_timingTasks.find(taskName);
    if (it == m_timingTasks.end() || !it->second.m_isRunning)
    {
        return -1.0; // 任务不存在或未运行
    }

    auto currentTime = std::chrono::high_resolution_clock::now();
    return CalculateDuration(it->second.m_startTime, currentTime, unit);
}

/// <summary>
/// 检查任务是否正在计时
/// </summary>
/// <param name="taskName">任务名称</param>
/// <returns>如果任务正在计时返回true，否则返回false</returns>
bool TimeSystem::IsTimingActive(const std::string& taskName) const
{
    if (taskName.empty())
    {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_timingTasks.find(taskName);
    return (it != m_timingTasks.end() && it->second.m_isRunning);
}

/// <summary>
/// 取消计时任务
/// </summary>
/// <param name="taskName">任务名称</param>
/// <returns>如果成功取消返回true，否则返回false</returns>
bool TimeSystem::CancelTiming(const std::string& taskName)
{
    if (taskName.empty())
    {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_timingTasks.find(taskName);
    if (it == m_timingTasks.end())
    {
        return false; // 任务不存在
    }

    m_timingTasks.erase(it);
    return true;
}

/// <summary>
/// 获取当前高精度时间戳
/// </summary>
/// <returns>当前时间点</returns>
std::chrono::high_resolution_clock::time_point TimeSystem::GetCurrentTimePoint() const
{
    return std::chrono::high_resolution_clock::now();
}

/// <summary>
/// 获取当前Unix时间戳（秒）
/// </summary>
/// <returns>Unix时间戳</returns>
std::int64_t TimeSystem::GetCurrentUnixTimestamp() const
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
}

/// <summary>
/// 获取当前Unix时间戳（毫秒）
/// </summary>
/// <returns>Unix时间戳（毫秒）</returns>
std::int64_t TimeSystem::GetCurrentUnixTimestampMs() const
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

/// <summary>
/// 清除所有计时任务
/// </summary>
void TimeSystem::ClearAllTimings()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_timingTasks.clear();
}

/// <summary>
/// 获取活跃计时任务数量
/// </summary>
/// <returns>活跃任务数量</returns>
size_t TimeSystem::GetActiveTimingCount() const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    size_t activeCount = 0;
    for (const auto& pair : m_timingTasks)
    {
        if (pair.second.m_isRunning)
        {
            ++activeCount;
        }
    }

    return activeCount;
}

/// <summary>
/// 计算两个时间点之间的持续时间
/// </summary>
/// <param name="startTime">开始时间</param>
/// <param name="endTime">结束时间</param>
/// <param name="unit">时间单位</param>
/// <returns>持续时间</returns>
double TimeSystem::CalculateDuration(const std::chrono::high_resolution_clock::time_point& startTime, const std::chrono::high_resolution_clock::time_point& endTime, EM_TimeUnit unit) const
{
    auto duration = endTime - startTime;

    switch (unit)
    {
        case EM_TimeUnit::Nanoseconds:
            return static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
        case EM_TimeUnit::Microseconds:
            return static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(duration).count());
        case EM_TimeUnit::Milliseconds:
            return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
        case EM_TimeUnit::Seconds:
            return std::chrono::duration<double>(duration).count();
        case EM_TimeUnit::Minutes:
            return std::chrono::duration<double, std::ratio<60>>(duration).count();
        case EM_TimeUnit::Hours:
            return std::chrono::duration<double, std::ratio<3600>>(duration).count();
        default:
            return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
    }
}

/// <summary>
/// 获取时间单位字符串
/// </summary>
/// <param name="unit">时间单位</param>
/// <returns>单位字符串</returns>
std::string TimeSystem::GetTimeUnitString(EM_TimeUnit unit) const
{
    switch (unit)
    {
        case EM_TimeUnit::Nanoseconds:
            return "ns";
        case EM_TimeUnit::Microseconds:
            return "μs";
        case EM_TimeUnit::Milliseconds:
            return "ms";
        case EM_TimeUnit::Seconds:
            return "s";
        case EM_TimeUnit::Minutes:
            return "min";
        case EM_TimeUnit::Hours:
            return "h";
        default:
            return "ms";
    }
}

/// <summary>
/// 格式化计时日志消息
/// </summary>
/// <param name="taskName">任务名称</param>
/// <param name="elapsedTime">耗时</param>
/// <param name="unit">时间单位</param>
/// <param name="customMessage">自定义消息</param>
/// <returns>格式化的日志消息</returns>
std::string TimeSystem::FormatTimingLogMessage(const std::string& taskName, double elapsedTime, EM_TimeUnit unit, const std::string& customMessage) const
{
    std::ostringstream oss;

    if (!customMessage.empty())
    {
        oss << customMessage << " - ";
    }

    oss << "任务 [" << taskName << "] 执行耗时: " << std::fixed << std::setprecision(3) << elapsedTime << GetTimeUnitString(unit);

    return oss.str();
}

/// <summary>
/// 将时间系统日志级别转换为日志系统日志级别
/// </summary>
/// <param name="level">时间系统日志级别</param>
/// <returns>对应的日志系统级别</returns>
int TimeSystem::ConvertToLogSystemLevel(EM_TimingLogLevel level) const
{
    switch (level)
    {
        case EM_TimingLogLevel::Debug:
            return 0; // Debug
        case EM_TimingLogLevel::Info:
            return 1; // Info
        case EM_TimingLogLevel::Warning:
            return 2; // Warning
        case EM_TimingLogLevel::Error:
            return 3; // Error
        case EM_TimingLogLevel::Fatal:
            return 4; // Fatal
        default:
            return 1; // Info
    }
}

/// <summary>
/// 输出日志到日志系统
/// </summary>
/// <param name="logLevel">日志级别</param>
/// <param name="message">日志消息</param>
void TimeSystem::WriteToLogSystem(EM_TimingLogLevel logLevel, const std::string& message)
{
    // 暂时输出到控制台，后续可以集成具体的日志系统
    std::string levelStr;
    switch (logLevel)
    {
        case EM_TimingLogLevel::Debug:
            levelStr = "[DEBUG]";
            break;
        case EM_TimingLogLevel::Info:
            levelStr = "[INFO]";
            break;
        case EM_TimingLogLevel::Warning:
            levelStr = "[WARNING]";
            break;
        case EM_TimingLogLevel::Error:
            levelStr = "[ERROR]";
            break;
        case EM_TimingLogLevel::Fatal:
            levelStr = "[FATAL]";
            break;
        default:
            levelStr = "[INFO]";
            break;
    }

    std::cout << levelStr << " " << message << std::endl;

    // TODO: 在这里集成具体的日志系统调用
    // LogSystem::Instance().WriteLog(static_cast<EM_LogLevel>(ConvertToLogSystemLevel(logLevel)), message);
}

// ============================================================================
// AutoTimer类实现
// ============================================================================

/// <summary>
/// 构造函数，自动开始计时
/// </summary>
/// <param name="taskName">任务名称</param>
/// <param name="logOnDestroy">析构时是否自动记录日志</param>
/// <param name="logLevel">日志级别</param>
/// <param name="unit">时间单位</param>
AutoTimer::AutoTimer(const std::string& taskName, bool logOnDestroy, EM_TimingLogLevel logLevel, EM_TimeUnit unit)
    : m_taskName(taskName), m_logOnDestroy(logOnDestroy), m_logLevel(logLevel), m_unit(unit), m_stopped(false)
{
    TimeSystem::Instance().StartTiming(m_taskName);
}

/// <summary>
/// 析构函数，自动停止计时
/// </summary>
AutoTimer::~AutoTimer()
{
    if (!m_stopped)
    {
        Stop();
    }
}

/// <summary>
/// 获取当前耗时
/// </summary>
/// <returns>当前耗时</returns>
double AutoTimer::GetElapsedTime() const
{
    if (m_stopped)
    {
        return -1.0;
    }

    return TimeSystem::Instance().GetElapsedTime(m_taskName, m_unit);
}

/// <summary>
/// 手动停止计时
/// </summary>
/// <returns>总耗时</returns>
double AutoTimer::Stop()
{
    if (m_stopped)
    {
        return -1.0;
    }
    
    double elapsedTime = -1.0;
    
    if (m_logOnDestroy)
    {
        elapsedTime = TimeSystem::Instance().StopTimingWithLog(m_taskName, m_logLevel, m_unit);
    }
    else
    {
        elapsedTime = TimeSystem::Instance().StopTiming(m_taskName, m_unit);
    }
    
    m_stopped = true;
    return elapsedTime;
}

/// <summary>
/// 批量开始多个相关计时任务
/// </summary>
/// <param name="taskNames">任务名称列表</param>
/// <param name="prefix">任务名称前缀</param>
/// <returns>成功开始的任务数量</returns>
size_t TimeSystem::StartBatchTiming(const std::vector<std::string>& taskNames, const std::string& prefix)
{
    if (taskNames.empty())
    {
        return 0;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    
    size_t successCount = 0;
    for (const auto& taskName : taskNames)
    {
        std::string fullTaskName = prefix.empty() ? taskName : prefix + "_" + taskName;
        
        // 检查任务是否已经存在
        auto it = m_timingTasks.find(fullTaskName);
        if (it != m_timingTasks.end() && it->second.m_isRunning)
        {
            continue; // 任务已经在运行，跳过
        }

        // 创建新的计时任务
        m_timingTasks[fullTaskName] = ST_TimingInfo(fullTaskName);
        ++successCount;
    }
    
    return successCount;
}

/// <summary>
/// 批量停止多个相关计时任务并记录日志
/// </summary>
/// <param name="taskNames">任务名称列表</param>
/// <param name="prefix">任务名称前缀</param>
/// <param name="logLevel">日志级别</param>
/// <returns>任务耗时映射表</returns>
std::unordered_map<std::string, double> TimeSystem::StopBatchTimingWithLog(const std::vector<std::string>& taskNames, const std::string& prefix, EM_TimingLogLevel logLevel)
{
    std::unordered_map<std::string, double> results;
    
    if (taskNames.empty())
    {
        return results;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for (const auto& taskName : taskNames)
    {
        std::string fullTaskName = prefix.empty() ? taskName : prefix + "_" + taskName;
        
        auto it = m_timingTasks.find(fullTaskName);
        if (it == m_timingTasks.end() || !it->second.m_isRunning)
        {
            results[fullTaskName] = -1.0; // 任务不存在或未运行
            continue;
        }

        double elapsedTime = CalculateDuration(it->second.m_startTime, endTime, EM_TimeUnit::Milliseconds);
        results[fullTaskName] = elapsedTime;
        
        // 记录日志
        std::string logMessage = FormatTimingLogMessage(fullTaskName, elapsedTime, EM_TimeUnit::Milliseconds, "");
        WriteToLogSystem(logLevel, logMessage);
        
        // 移除任务
        m_timingTasks.erase(it);
    }
    
    return results;
}

/// <summary>
/// 获取系统启动以来的高精度时间戳（微秒）
/// </summary>
/// <returns>微秒时间戳</returns>
std::int64_t TimeSystem::GetHighResolutionTimestamp() const
{
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

/// <summary>
/// 计算两个时间点之间的时间差
/// </summary>
/// <param name="startTime">开始时间点</param>
/// <param name="endTime">结束时间点</param>
/// <param name="unit">时间单位</param>
/// <returns>时间差</returns>
double TimeSystem::CalculateTimeDifference(const std::chrono::high_resolution_clock::time_point& startTime, const std::chrono::high_resolution_clock::time_point& endTime, EM_TimeUnit unit) const
{
    return CalculateDuration(startTime, endTime, unit);
}

/// <summary>
/// 获取格式化的当前时间字符串
/// </summary>
/// <param name="format">时间格式字符串</param>
/// <returns>格式化的时间字符串</returns>
std::string TimeSystem::GetFormattedCurrentTime(const std::string& format) const
{
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::tm tm = {};
#ifdef _WIN32
    localtime_s(&tm, &time_t);
#else
    localtime_r(&time_t, &tm);
#endif
    
    char buffer[256] = {0};
    std::strftime(buffer, sizeof(buffer), format.c_str(), &tm);
    
    return std::string(buffer);
}

/// <summary>
/// 休眠指定时间
/// </summary>
/// <param name="duration">休眠时长</param>
/// <param name="unit">时间单位</param>
void TimeSystem::Sleep(double duration, EM_TimeUnit unit)
{
    if (duration <= 0.0)
    {
        return;
    }
    
    switch (unit)
    {
        case EM_TimeUnit::Nanoseconds:
            std::this_thread::sleep_for(std::chrono::nanoseconds(static_cast<int64_t>(duration)));
            break;
        case EM_TimeUnit::Microseconds:
            std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int64_t>(duration)));
            break;
        case EM_TimeUnit::Milliseconds:
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int64_t>(duration)));
            break;
        case EM_TimeUnit::Seconds:
            std::this_thread::sleep_for(std::chrono::duration<double>(duration));
            break;
        case EM_TimeUnit::Minutes:
            std::this_thread::sleep_for(std::chrono::duration<double, std::ratio<60>>(duration));
            break;
        case EM_TimeUnit::Hours:
            std::this_thread::sleep_for(std::chrono::duration<double, std::ratio<3600>>(duration));
            break;
        default:
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int64_t>(duration)));
            break;
    }
}
