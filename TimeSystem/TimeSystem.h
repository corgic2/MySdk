/// <summary>
/// 时间系统类头文件 - 基于C++17时间库的计时功能封装
/// </summary>
#pragma once

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>
#include <ctime>
#include "../SDKCommonDefine/SDK_Export.h"

/// <summary>
/// 简化的日志级别枚举（用于时间系统）
/// </summary>
enum class EM_TimingLogLevel
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
/// 时间单位枚举
/// </summary>
enum class EM_TimeUnit
{
    Nanoseconds,
    ///< 纳秒
    Microseconds,
    ///< 微秒
    Milliseconds,
    ///< 毫秒
    Seconds,
    ///< 秒
    Minutes,
    ///< 分钟
    Hours ///< 小时
};

/// <summary>
/// 计时信息结构体
/// </summary>
struct ST_TimingInfo
{
    std::chrono::high_resolution_clock::time_point m_startTime; ///< 开始时间点
    std::string m_taskName;                                     ///< 任务名称
    bool m_isRunning;                                           ///< 是否正在运行

    /// <summary>
    /// 构造函数，初始化默认值
    /// </summary>
    ST_TimingInfo()
        : m_startTime(std::chrono::high_resolution_clock::now()), m_taskName(""), m_isRunning(false)
    {
    }

    /// <summary>
    /// 构造函数
    /// </summary>
    /// <param name="taskName">任务名称</param>
    ST_TimingInfo(const std::string& taskName)
        : m_startTime(std::chrono::high_resolution_clock::now()), m_taskName(taskName), m_isRunning(true)
    {
    }
};

/// <summary>
/// 时间系统类 - 提供高精度计时功能
/// </summary>
class SDK_API TimeSystem
{
public:
    /// <summary>
    /// 获取时间系统单例
    /// </summary>
    /// <returns>时间系统单例引用</returns>
    static TimeSystem& Instance();

    /// <summary>
    /// 开始计时
    /// </summary>
    /// <param name="taskName">任务名称，用于标识不同的计时任务</param>
    /// <returns>如果成功开始计时返回true，否则返回false</returns>
    bool StartTiming(const std::string& taskName);

    /// <summary>
    /// 终止计时并返回耗时
    /// </summary>
    /// <param name="taskName">任务名称</param>
    /// <param name="unit">返回的时间单位</param>
    /// <returns>任务耗时，如果任务不存在返回-1</returns>
    double StopTiming(const std::string& taskName, EM_TimeUnit unit = EM_TimeUnit::Milliseconds);

    /// <summary>
    /// 终止计时并通过日志系统输出耗时信息
    /// </summary>
    /// <param name="taskName">任务名称</param>
    /// <param name="logLevel">日志级别</param>
    /// <param name="unit">时间单位</param>
    /// <param name="customMessage">自定义消息前缀</param>
    /// <returns>任务耗时，如果任务不存在返回-1</returns>
    double StopTimingWithLog(const std::string& taskName, EM_TimingLogLevel logLevel = EM_TimingLogLevel::Info, EM_TimeUnit unit = EM_TimeUnit::Milliseconds, const std::string& customMessage = "");

    /// <summary>
    /// 获取正在进行的任务耗时（不停止计时）
    /// </summary>
    /// <param name="taskName">任务名称</param>
    /// <param name="unit">时间单位</param>
    /// <returns>当前耗时，如果任务不存在返回-1</returns>
    double GetElapsedTime(const std::string& taskName, EM_TimeUnit unit = EM_TimeUnit::Milliseconds) const;

    /// <summary>
    /// 检查任务是否正在计时
    /// </summary>
    /// <param name="taskName">任务名称</param>
    /// <returns>如果任务正在计时返回true，否则返回false</returns>
    bool IsTimingActive(const std::string& taskName) const;

    /// <summary>
    /// 取消计时任务
    /// </summary>
    /// <param name="taskName">任务名称</param>
    /// <returns>如果成功取消返回true，否则返回false</returns>
    bool CancelTiming(const std::string& taskName);

    /// <summary>
    /// 获取当前高精度时间戳
    /// </summary>
    /// <returns>当前时间点</returns>
    std::chrono::high_resolution_clock::time_point GetCurrentTimePoint() const;

    /// <summary>
    /// 获取当前Unix时间戳（秒）
    /// </summary>
    /// <returns>Unix时间戳</returns>
    std::int64_t GetCurrentUnixTimestamp() const;

    /// <summary>
    /// 获取当前Unix时间戳（毫秒）
    /// </summary>
    /// <returns>Unix时间戳（毫秒）</returns>
    std::int64_t GetCurrentUnixTimestampMs() const;

    /// <summary>
    /// 清除所有计时任务
    /// </summary>
    void ClearAllTimings();

    /// <summary>
    /// 获取活跃计时任务数量
    /// </summary>
    /// <returns>活跃任务数量</returns>
    size_t GetActiveTimingCount() const;

    /// <summary>
    /// 批量开始多个相关计时任务
    /// </summary>
    /// <param name="taskNames">任务名称列表</param>
    /// <param name="prefix">任务名称前缀</param>
    /// <returns>成功开始的任务数量</returns>
    size_t StartBatchTiming(const std::vector<std::string>& taskNames, const std::string& prefix = "");

    /// <summary>
    /// 批量停止多个相关计时任务并记录日志
    /// </summary>
    /// <param name="taskNames">任务名称列表</param>
    /// <param name="prefix">任务名称前缀</param>
    /// <param name="logLevel">日志级别</param>
    /// <returns>任务耗时映射表</returns>
    std::unordered_map<std::string, double> StopBatchTimingWithLog(const std::vector<std::string>& taskNames, const std::string& prefix = "", EM_TimingLogLevel logLevel = EM_TimingLogLevel::Info);

    /// <summary>
    /// 获取系统启动以来的高精度时间戳（微秒）
    /// </summary>
    /// <returns>微秒时间戳</returns>
    std::int64_t GetHighResolutionTimestamp() const;

    /// <summary>
    /// 计算两个时间点之间的时间差
    /// </summary>
    /// <param name="startTime">开始时间点</param>
    /// <param name="endTime">结束时间点</param>
    /// <param name="unit">时间单位</param>
    /// <returns>时间差</returns>
    double CalculateTimeDifference(const std::chrono::high_resolution_clock::time_point& startTime, const std::chrono::high_resolution_clock::time_point& endTime, EM_TimeUnit unit = EM_TimeUnit::Milliseconds) const;

    /// <summary>
    /// 获取格式化的当前时间字符串
    /// </summary>
    /// <param name="format">时间格式字符串</param>
    /// <returns>格式化的时间字符串</returns>
    std::string GetFormattedCurrentTime(const std::string& format = "%Y-%m-%d %H:%M:%S") const;

    /// <summary>
    /// 休眠指定时间
    /// </summary>
    /// <param name="duration">休眠时长</param>
    /// <param name="unit">时间单位</param>
    static void Sleep(double duration, EM_TimeUnit unit = EM_TimeUnit::Milliseconds);

    /// <summary>
    /// 析构函数
    /// </summary>
    ~TimeSystem();

private:
    /// <summary>
    /// 构造函数
    /// </summary>
    TimeSystem();
    TimeSystem(const TimeSystem&) = delete;
    TimeSystem& operator=(const TimeSystem&) = delete;

    /// <summary>
    /// 计算两个时间点之间的持续时间
    /// </summary>
    /// <param name="startTime">开始时间</param>
    /// <param name="endTime">结束时间</param>
    /// <param name="unit">时间单位</param>
    /// <returns>持续时间</returns>
    double CalculateDuration(const std::chrono::high_resolution_clock::time_point& startTime, const std::chrono::high_resolution_clock::time_point& endTime, EM_TimeUnit unit) const;

    /// <summary>
    /// 获取时间单位字符串
    /// </summary>
    /// <param name="unit">时间单位</param>
    /// <returns>单位字符串</returns>
    std::string GetTimeUnitString(EM_TimeUnit unit) const;

    /// <summary>
    /// 格式化计时日志消息
    /// </summary>
    /// <param name="taskName">任务名称</param>
    /// <param name="elapsedTime">耗时</param>
    /// <param name="unit">时间单位</param>
    /// <param name="customMessage">自定义消息</param>
    /// <returns>格式化的日志消息</returns>
    std::string FormatTimingLogMessage(const std::string& taskName, double elapsedTime, EM_TimeUnit unit, const std::string& customMessage) const;

    /// <summary>
    /// 将时间系统日志级别转换为日志系统日志级别
    /// </summary>
    /// <param name="level">时间系统日志级别</param>
    /// <returns>对应的日志系统级别</returns>
    int ConvertToLogSystemLevel(EM_TimingLogLevel level) const;

    /// <summary>
    /// 输出日志到日志系统
    /// </summary>
    /// <param name="logLevel">日志级别</param>
    /// <param name="message">日志消息</param>
    void WriteToLogSystem(EM_TimingLogLevel logLevel, const std::string& message);

private:
    std::unordered_map<std::string, ST_TimingInfo> m_timingTasks; ///< 计时任务映射表
    mutable std::mutex m_mutex;                                   ///< 线程安全互斥锁
};

/// <summary>
/// RAII风格的自动计时器类
/// </summary>
class SDK_API AutoTimer
{
public:
    /// <summary>
    /// 构造函数，自动开始计时
    /// </summary>
    /// <param name="taskName">任务名称</param>
    /// <param name="logOnDestroy">析构时是否自动记录日志</param>
    /// <param name="logLevel">日志级别</param>
    /// <param name="unit">时间单位</param>
    explicit AutoTimer(const std::string& taskName, bool logOnDestroy = true, EM_TimingLogLevel logLevel = EM_TimingLogLevel::Info, EM_TimeUnit unit = EM_TimeUnit::Milliseconds);

    /// <summary>
    /// 析构函数，自动停止计时
    /// </summary>
    ~AutoTimer();

    /// <summary>
    /// 获取当前耗时
    /// </summary>
    /// <returns>当前耗时</returns>
    double GetElapsedTime() const;

    /// <summary>
    /// 手动停止计时
    /// </summary>
    /// <returns>总耗时</returns>
    double Stop();

private:
    std::string m_taskName;       ///< 任务名称
    bool m_logOnDestroy;          ///< 析构时是否记录日志
    EM_TimingLogLevel m_logLevel; ///< 日志级别
    EM_TimeUnit m_unit;           ///< 时间单位
    bool m_stopped;               ///< 是否已停止
};

/// <summary>
/// 便捷的计时宏定义
/// </summary>
#define TIME_START(taskName) TimeSystem::Instance().StartTiming(taskName)
#define TIME_STOP(taskName) TimeSystem::Instance().StopTiming(taskName)
#define TIME_STOP_LOG(taskName, logLevel) TimeSystem::Instance().StopTimingWithLog(taskName, logLevel)
#define AUTO_TIMER(taskName) AutoTimer __autoTimer(taskName)
#define AUTO_TIMER_LOG(taskName, logLevel) AutoTimer __autoTimer(taskName, true, logLevel)

/// <summary>
/// 批量计时宏定义
/// </summary>
#define TIME_BATCH_START(taskNames, prefix) TimeSystem::Instance().StartBatchTiming(taskNames, prefix)
#define TIME_BATCH_STOP_LOG(taskNames, prefix, logLevel) TimeSystem::Instance().StopBatchTimingWithLog(taskNames, prefix, logLevel)

/// <summary>
/// 音视频专用计时宏
/// </summary>
#define AV_TIMER_START(module, task) TIME_START(module "_" task)
#define AV_TIMER_STOP_LOG(module, task) TIME_STOP_LOG(module "_" task, EM_TimingLogLevel::Info)
#define AV_AUTO_TIMER(module, task) AUTO_TIMER_LOG(module "_" task, EM_TimingLogLevel::Info)

/// <summary>
/// 睡眠宏定义
/// </summary>
#define TIME_SLEEP_MS(ms) TimeSystem::Sleep(ms, EM_TimeUnit::Milliseconds)
#define TIME_SLEEP_US(us) TimeSystem::Sleep(us, EM_TimeUnit::Microseconds)
#define TIME_SLEEP_S(s) TimeSystem::Sleep(s, EM_TimeUnit::Seconds)
