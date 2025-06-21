#ifdef _WIN32
#define NOMINMAX  // 防止 Windows 定义 min/max 宏
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

/// <summary>
/// 主程序入口
/// </summary>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include "LogSystem/LogSystem.h"
#include "ThreadPool/ThreadPool.h"

/// <summary>
/// 性能测试结果结构体
/// </summary>
struct ST_PerfTestResult
{
    double avgLatency;   ///< 平均延迟（毫秒）
    double maxLatency;   ///< 最大延迟（毫秒）
    double minLatency;   ///< 最小延迟（毫秒）
    double throughput;   ///< 吞吐量（任务/秒）
    double cpuUsage;     ///< CPU使用率（%）
    size_t successTasks; ///< 成功完成的任务数
    size_t failedTasks;  ///< 失败的任务数
};

/// <summary>
/// 获取当前CPU使用率
/// </summary>
double GetCPUUsage()
{
#ifdef _WIN32
    FILETIME idleTime, kernelTime, userTime;
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime))
    {
        return 0.0;
    }

    static auto FileTimeToInt64 = [](const FILETIME& ft)
    {
        return (static_cast<uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
    };

    static uint64_t lastIdleTime = FileTimeToInt64(idleTime);
    static uint64_t lastKernelTime = FileTimeToInt64(kernelTime);
    static uint64_t lastUserTime = FileTimeToInt64(userTime);

    uint64_t idle = FileTimeToInt64(idleTime);
    uint64_t kernel = FileTimeToInt64(kernelTime);
    uint64_t user = FileTimeToInt64(userTime);

    uint64_t idleDiff = idle - lastIdleTime;
    uint64_t totalDiff = (kernel + user) - (lastKernelTime + lastUserTime);

    lastIdleTime = idle;
    lastKernelTime = kernel;
    lastUserTime = user;

    return totalDiff > 0 ? (1.0 - static_cast<double>(idleDiff) / totalDiff) * 100.0 : 0.0;
#else
    return 0.0; // 非Windows系统暂不支持
#endif
}

/// <summary>
/// 执行压力测试
/// </summary>
/// <param name="pool">线程池实例</param>
/// <param name="taskCount">总任务数</param>
/// <param name="taskDuration">任务执行时间（毫秒）</param>
/// <param name="priorityMix">是否混合使用不同优先级</param>
/// <returns>测试结果</returns>
ST_PerfTestResult RunStressTest(ThreadPool& pool, size_t taskCount, int taskDuration, bool priorityMix)
{
    // 使用智能指针在堆上分配大型容器
    auto results = std::make_unique<std::vector<std::future<std::chrono::milliseconds>>>();
    auto latencies = std::make_unique<std::vector<std::chrono::milliseconds>>();

    // 预分配空间以避免重新分配
    results->reserve(taskCount);
    latencies->reserve(taskCount);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> priorityDist(0, 3);

    auto start = std::chrono::high_resolution_clock::now();
    double initialCPU = GetCPUUsage();

    // 分批提交任务，避免一次性创建过多任务
    const size_t BATCH_SIZE = 1000;
    for (size_t i = 0; i < taskCount; i += BATCH_SIZE)
    {
        size_t currentBatchSize = std::min(BATCH_SIZE, taskCount - i);

        for (size_t j = 0; j < currentBatchSize; ++j)
        {
            auto priority = priorityMix ? static_cast<EM_TaskPriority>(priorityDist(gen)) : EM_TaskPriority::Normal;

            auto future = pool.Submit([taskDuration]() -> std::chrono::milliseconds
            {
                auto taskStart = std::chrono::high_resolution_clock::now();

                // 减少栈上的计算量
                volatile int dummy = 0;
                for (int k = 0; k < taskDuration * 100; ++k)
                {
                    dummy += k;
                }

                auto taskEnd = std::chrono::high_resolution_clock::now();
                return std::chrono::duration_cast<std::chrono::milliseconds>(taskEnd - taskStart);
            }, priority);

            results->push_back(std::move(future));
        }

        // 每批次处理完后让出一些时间给其他线程
        std::this_thread::yield();
    }

    // 分批处理结果
    size_t successCount = 0;
    size_t failCount = 0;
    double maxLat = 0;
    double minLat = (std::numeric_limits<double>::max)();
    double totalLat = 0;

    for (auto& result : *results)
    {
        try
        {
            auto latency = result.get();
            double latencyMs = latency.count();
            latencies->push_back(latency);
            maxLat = (std::max)(maxLat, latencyMs);
            minLat = (std::min)(minLat, latencyMs);
            totalLat += latencyMs;
            ++successCount;

            // 定期让出CPU
            if (successCount % 1000 == 0)
            {
                std::this_thread::yield();
            }
        } catch (...)
        {
            ++failCount;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    double finalCPU = GetCPUUsage();

    ST_PerfTestResult result;
    result.avgLatency = successCount > 0 ? totalLat / successCount : 0;
    result.maxLatency = maxLat;
    result.minLatency = minLat;
    result.throughput = (successCount * 1000.0) / duration.count();
    result.cpuUsage = (finalCPU + initialCPU) / 2.0;
    result.successTasks = successCount;
    result.failedTasks = failCount;

    return result;
}

/// <summary>
/// 打印测试结果
/// </summary>
void PrintTestResult(const std::string& testName, const ST_PerfTestResult& result)
{
    std::cout << "\n=== " << testName << " ===\n" << std::fixed << std::setprecision(2) << "平均延迟: " << result.avgLatency << " ms\n" << "最大延迟: " << result.maxLatency << " ms\n" << "最小延迟: " << result.minLatency << " ms\n" << "吞吐量: " << result.throughput << " 任务/秒\n" << "CPU使用率: " << result.cpuUsage << "%\n" << "成功任务: " << result.successTasks << "\n" << "失败任务: " << result.failedTasks << "\n" << std::endl;
}

/// <summary>
/// 执行线程池压力测试
/// </summary>
void StressTestThreadPool()
{
    std::cout << "\n=== 开始线程池压力测试 ===\n" << std::endl;

    // 减小测试规模，避免栈溢出
    const size_t TASK_COUNT = 10000; // 从100000减少到10000
    const int TASK_DURATION = 1;     // 毫秒

    try
    {
        // 创建不同配置的线程池
        ST_ThreadPoolConfig smallConfig;
        smallConfig.m_minThreads = 2;
        smallConfig.m_maxThreads = 4;
        ThreadPool smallPool(smallConfig);

        // 运行单一配置测试
        std::cout << "执行单一优先级测试 (小型线程池)..." << std::endl;
        auto smallResult = RunStressTest(smallPool, TASK_COUNT, TASK_DURATION, false);
        PrintTestResult("小型线程池 (2-4线程)", smallResult);
        smallPool.Shutdown();

        // 等待一段时间后继续下一个测试
        std::this_thread::sleep_for(std::chrono::seconds(1));

        ST_ThreadPoolConfig mediumConfig;
        mediumConfig.m_minThreads = 4;
        mediumConfig.m_maxThreads = 8;
        ThreadPool mediumPool(mediumConfig);

        std::cout << "执行单一优先级测试 (中型线程池)..." << std::endl;
        auto mediumResult = RunStressTest(mediumPool, TASK_COUNT, TASK_DURATION, false);
        PrintTestResult("中型线程池 (4-8线程)", mediumResult);

        std::cout << "\n执行混合优先级测试..." << std::endl;
        auto mixedResult = RunStressTest(mediumPool, TASK_COUNT, TASK_DURATION, true);
        PrintTestResult("混合优先级测试 (4-8线程)", mixedResult);
        mediumPool.Shutdown();

        // 等待一段时间后继续下一个测试
        std::this_thread::sleep_for(std::chrono::seconds(1));

        ST_ThreadPoolConfig largeConfig;
        largeConfig.m_minThreads = 8;
        largeConfig.m_maxThreads = 16;
        ThreadPool largePool(largeConfig);

        std::cout << "执行单一优先级测试 (大型线程池)..." << std::endl;
        auto largeResult = RunStressTest(largePool, TASK_COUNT, TASK_DURATION, false);
        PrintTestResult("大型线程池 (8-16线程)", largeResult);
        largePool.Shutdown();
    } catch (const std::exception& e)
    {
        std::cerr << "压力测试异常: " << e.what() << std::endl;
    }
}

/// <summary>
/// 测试日志系统功能
/// </summary>
void TestLogSystem()
{
    std::cout << "\n=== 测试日志系统 ===\n" << std::endl;

    // 配置日志系统
    ST_LogConfig logConfig;
    logConfig.m_logFilePath = "test.log";
    logConfig.m_logLevel = EM_LogLevel::Debug;
    logConfig.m_maxFileSize = 1024 * 1024; // 1MB
    logConfig.m_maxQueueSize = 10000;
    logConfig.m_asyncEnabled = true;

    LogSystem::Instance().Initialize(logConfig);

    // 测试不同级别的日志
    LOG_DEBUG("这是一条调试信息");
    LOG_INFO("正在处理用户请求: {}", "登录");
    LOG_WARN("系统资源使用率较高: {}%", 85);
    LOG_ERROR("数据库连接失败: {}", "超时");
    LOG_FATAL("检测到系统崩溃: {}", "内存损坏");

    // 测试高并发日志
    std::cout << "开始高并发日志测试..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    const int messageCount = 1000000;
    std::vector<std::thread> threads;
    for (int t = 0; t < 4; ++t)
    {
        threads.emplace_back([t, messageCount]()
        {
            for (int i = t * (messageCount / 4); i < (t + 1) * (messageCount / 4); ++i)
            {
                LOG_INFO("线程 {} - 并发日志消息 #{}", t, i);
            }
        });
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "写入 " << messageCount << " 条日志消息，耗时 " << duration.count() << "ms" << std::endl;
    std::cout << "平均吞吐量: " << (messageCount * 1.0 / duration.count()) << " 条/ms" << std::endl;

    // 确保所有日志都写入文件
    LogSystem::Instance().Flush();
    LogSystem::Instance().Shutdown();
}

/// <summary>
/// 测试线程池功能
/// </summary>
void TestThreadPool()
{
    std::cout << "\n=== 测试线程池 ===\n" << std::endl;

    // 创建线程池配置
    ST_ThreadPoolConfig config;
    config.m_minThreads = 2;
    config.m_maxThreads = 8;
    config.m_maxQueueSize = 1000;
    config.m_keepAliveTime = 1000; // 1秒

    // 创建线程池
    ThreadPool pool(config);

    // 创建一些测试任务
    std::vector<std::future<int>> results;

    std::cout << "开始提交任务..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    // 添加计算密集型任务
    for (int i = 0; i < 100; ++i)
    {
        auto future = pool.Submit([i]() -> int
        {
            // 模拟计算密集型任务
            int sum = 0;
            for (int j = 0; j < 1000000; ++j)
            {
                sum += j * i;
            }
            return sum;
        }, EM_TaskPriority::Normal);

        results.push_back(std::move(future));
    }

    // 添加一些高优先级任务
    for (int i = 0; i < 10; ++i)
    {
        auto future = pool.Submit([i]() -> int
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return i * 100;
        }, EM_TaskPriority::High);

        results.push_back(std::move(future));
    }

    std::cout << "等待任务完成..." << std::endl;

    // 等待所有任务完成并检查结果
    int completedTasks = 0;
    for (auto& result : results)
    {
        try
        {
            result.get();
            ++completedTasks;
        } catch (const std::exception& e)
        {
            std::cerr << "任务执行失败: " << e.what() << std::endl;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "完成 " << completedTasks << " 个任务，耗时 " << duration.count() << "ms" << std::endl;
    std::cout << "活动线程数: " << pool.GetCurrentThreadCount() << std::endl;
    std::cout << "等待任务数: " << pool.GetTaskCount() << std::endl;

    // 测试线程池调整大小
    std::cout << "\n测试线程池大小调整..." << std::endl;
    pool.Resize(4, 16);
    std::cout << "调整后活动线程数: " << pool.GetCurrentThreadCount() << std::endl;

    // 等待所有任务完成
    pool.WaitAll();
    std::cout << "所有任务已完成" << std::endl;

    // 优雅关闭线程池
    pool.Shutdown();
    std::cout << "线程池已关闭" << std::endl;
}

/// <summary>
/// 执行简单的计算密集型任务测试
/// </summary>
void TestComputeBoundTasks()
{
    std::cout << "\n=== 计算密集型任务测试 ===\n" << std::endl;

    // 创建线程池
    ST_ThreadPoolConfig config;
    config.m_minThreads = 4;
    config.m_maxThreads = 8;
    ThreadPool pool(config);

    // 增加任务数和计算量
    const int TASK_COUNT = 10000;
    const int COMPUTE_ITERATIONS = 1000000;
    std::vector<std::future<uint64_t>> results;
    results.reserve(TASK_COUNT);

    auto start = std::chrono::high_resolution_clock::now();

    // 提交计算密集型任务
    for (int i = 0; i < TASK_COUNT; ++i)
    {
        auto future = pool.Submit([i, COMPUTE_ITERATIONS]() -> uint64_t
        {
            uint64_t sum = 0;
            // 增加计算复杂度
            for (int j = 0; j < COMPUTE_ITERATIONS; ++j)
            {
                sum += (j * i) % 1000;
            }
            return sum;
        }, EM_TaskPriority::Normal);

        results.push_back(std::move(future));

        // 每1000个任务输出一次进度
        if ((i + 1) % 1000 == 0)
        {
            std::cout << "已提交: " << (i + 1) << "/" << TASK_COUNT << " 任务" << std::endl;
        }
    }

    // 等待所有任务完成并统计结果
    size_t completed = 0;
    size_t errors = 0;
    auto lastProgress = std::chrono::steady_clock::now();

    for (auto& result : results)
    {
        try
        {
            result.get();
            ++completed;

            // 每1000个任务或者超过1秒输出一次进度
            auto now = std::chrono::steady_clock::now();
            if (completed % 1000 == 0 || std::chrono::duration_cast<std::chrono::seconds>(now - lastProgress).count() >= 1)
            {
                std::cout << "已完成: " << completed << "/" << TASK_COUNT << " (错误: " << errors << ")" << std::endl;
                lastProgress = now;
            }
        } catch (const std::exception& e)
        {
            std::cerr << "任务失败: " << e.what() << std::endl;
            ++errors;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "\n=== 测试结果 ===" << std::endl;
    std::cout << "完成任务数: " << completed << "/" << TASK_COUNT << std::endl;
    std::cout << "错误任务数: " << errors << std::endl;
    std::cout << "总耗时: " << duration.count() << "ms" << std::endl;
    std::cout << "平均每任务耗时: " << (duration.count() * 1.0 / completed) << "ms" << std::endl;
    std::cout << "吞吐量: " << (completed * 1000.0 / duration.count()) << " 任务/秒" << std::endl;
    std::cout << "CPU线程数: " << pool.GetCurrentThreadCount() << std::endl;

    pool.Shutdown();
}

/// <summary>
/// 执行IO密集型任务测试
/// </summary>
void TestIOBoundTasks()
{
    std::cout << "\n=== IO密集型任务测试 ===\n" << std::endl;

    // 创建线程池，增加最大线程数
    ST_ThreadPoolConfig config;
    config.m_minThreads = 8;
    config.m_maxThreads = 32; // IO密集型任务需要更多线程
    ThreadPool pool(config);

    // 增加任务数
    const int TASK_COUNT = 10000;
    const int IO_SIMULATION_TIME = 20; // 模拟IO时间（毫秒）
    std::vector<std::future<std::chrono::milliseconds>> results;
    results.reserve(TASK_COUNT);

    auto start = std::chrono::high_resolution_clock::now();

    // 提交IO密集型任务
    for (int i = 0; i < TASK_COUNT; ++i)
    {
        auto future = pool.Submit([i, IO_SIMULATION_TIME]() -> std::chrono::milliseconds
        {
            auto taskStart = std::chrono::high_resolution_clock::now();

            // 模拟随机IO操作
            int sleepTime = IO_SIMULATION_TIME + (i % 10); // 添加一些随机性
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));

            // 模拟一些轻量计算
            volatile int dummy = 0;
            for (int j = 0; j < 1000; ++j)
            {
                dummy += j;
            }

            auto taskEnd = std::chrono::high_resolution_clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(taskEnd - taskStart);
        }, EM_TaskPriority::Normal);

        results.push_back(std::move(future));

        if ((i + 1) % 1000 == 0)
        {
            std::cout << "已提交: " << (i + 1) << "/" << TASK_COUNT << " 任务" << std::endl;
        }
    }

    // 统计结果
    size_t completed = 0;
    size_t errors = 0;
    double totalLatency = 0;
    double maxLatency = 0;
    auto lastProgress = std::chrono::steady_clock::now();

    for (auto& result : results)
    {
        try
        {
            auto latency = result.get();
            ++completed;
            double latencyMs = latency.count();
            totalLatency += latencyMs;
            maxLatency = std::max(maxLatency, latencyMs);

            auto now = std::chrono::steady_clock::now();
            if (completed % 1000 == 0 || std::chrono::duration_cast<std::chrono::seconds>(now - lastProgress).count() >= 1)
            {
                std::cout << "已完成: " << completed << "/" << TASK_COUNT << " (错误: " << errors << ")" << std::endl;
                lastProgress = now;
            }
        } catch (const std::exception& e)
        {
            std::cerr << "任务失败: " << e.what() << std::endl;
            ++errors;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "\n=== 测试结果 ===" << std::endl;
    std::cout << "完成任务数: " << completed << "/" << TASK_COUNT << std::endl;
    std::cout << "错误任务数: " << errors << std::endl;
    std::cout << "总耗时: " << duration.count() << "ms" << std::endl;
    std::cout << "平均每任务耗时: " << (duration.count() * 1.0 / completed) << "ms" << std::endl;
    std::cout << "平均任务延迟: " << (totalLatency / completed) << "ms" << std::endl;
    std::cout << "最大任务延迟: " << maxLatency << "ms" << std::endl;
    std::cout << "吞吐量: " << (completed * 1000.0 / duration.count()) << " 任务/秒" << std::endl;
    std::cout << "IO线程数: " << pool.GetCurrentThreadCount() << std::endl;

    pool.Shutdown();
}

/// <summary>
/// 执行混合优先级任务测试
/// </summary>
void TestMixedPriorityTasks()
{
    std::cout << "\n=== 混合优先级任务测试 ===\n" << std::endl;

    // 创建线程池
    ST_ThreadPoolConfig config;
    config.m_minThreads = 8;
    config.m_maxThreads = 16;
    ThreadPool pool(config);

    // 增加任务数和更均匀的优先级分布
    const int TASK_COUNT = 10000;
    const int TASK_DURATION = 5; // 基础任务时间（毫秒）
    std::vector<std::future<std::pair<int, std::chrono::milliseconds>>> results;
    results.reserve(TASK_COUNT);

    // 预定义优先级分布
    std::vector<EM_TaskPriority> priorities;
    priorities.reserve(TASK_COUNT);

    // 确保每个优先级的任务数量相等
    for (int i = 0; i < TASK_COUNT / 4; ++i)
    {
        priorities.push_back(EM_TaskPriority::Critical);
        priorities.push_back(EM_TaskPriority::High);
        priorities.push_back(EM_TaskPriority::Normal);
        priorities.push_back(EM_TaskPriority::Low);
    }

    // 打乱优先级顺序
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(priorities.begin(), priorities.end(), gen);

    auto start = std::chrono::high_resolution_clock::now();

    // 提交任务
    for (int i = 0; i < TASK_COUNT; ++i)
    {
        auto priority = priorities[i];
        auto future = pool.Submit([i, priority, TASK_DURATION]() -> std::pair<int, std::chrono::milliseconds>
        {
            auto taskStart = std::chrono::high_resolution_clock::now();

            // 模拟任务处理时间，高优先级任务处理时间略短
            int processingTime = TASK_DURATION;
            switch (priority)
            {
                case EM_TaskPriority::Critical:
                    processingTime = TASK_DURATION / 2;
                    break;
                case EM_TaskPriority::High:
                    processingTime = TASK_DURATION * 2 / 3;
                    break;
                case EM_TaskPriority::Normal:
                    processingTime = TASK_DURATION;
                    break;
                case EM_TaskPriority::Low:
                    processingTime = TASK_DURATION * 4 / 3;
                    break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(processingTime));

            auto taskEnd = std::chrono::high_resolution_clock::now();
            return std::make_pair(static_cast<int>(priority), std::chrono::duration_cast<std::chrono::milliseconds>(taskEnd - taskStart));
        }, priority);

        results.push_back(std::move(future));

        if ((i + 1) % 1000 == 0)
        {
            std::cout << "已提交: " << (i + 1) << "/" << TASK_COUNT << " 任务" << std::endl;
        }
    }

    // 统计结果
    std::array<int, 4> priorityCount = {0, 0, 0, 0};
    std::array<double, 4> priorityLatency = {0, 0, 0, 0};
    std::array<double, 4> maxLatency = {0, 0, 0, 0};
    size_t completed = 0;
    size_t errors = 0;
    auto lastProgress = std::chrono::steady_clock::now();

    for (auto& result : results)
    {
        try
        {
            auto [priority, latency] = result.get();
            ++completed;
            ++priorityCount[priority];
            double latencyMs = latency.count();
            priorityLatency[priority] += latencyMs;
            maxLatency[priority] = std::max(maxLatency[priority], latencyMs);

            auto now = std::chrono::steady_clock::now();
            if (completed % 1000 == 0 || std::chrono::duration_cast<std::chrono::seconds>(now - lastProgress).count() >= 1)
            {
                std::cout << "已完成: " << completed << "/" << TASK_COUNT << " (错误: " << errors << ")" << std::endl;
                lastProgress = now;
            }
        } catch (const std::exception& e)
        {
            std::cerr << "任务失败: " << e.what() << std::endl;
            ++errors;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "\n=== 测试结果 ===" << std::endl;
    std::cout << "完成任务数: " << completed << "/" << TASK_COUNT << std::endl;
    std::cout << "错误任务数: " << errors << std::endl;
    std::cout << "总耗时: " << duration.count() << "ms" << std::endl;
    std::cout << "平均每任务耗时: " << (duration.count() * 1.0 / completed) << "ms" << std::endl;
    std::cout << "吞吐量: " << (completed * 1000.0 / duration.count()) << " 任务/秒" << std::endl;
    std::cout << "\n各优先级任务统计:" << std::endl;

    const char* priorityNames[] = {"Critical", "High", "Normal", "Low"};
    for (int i = 0; i < 4; ++i)
    {
        if (priorityCount[i] > 0)
        {
            std::cout << priorityNames[i] << "优先级:" << "\n  完成数量: " << priorityCount[i] << "\n  平均延迟: " << (priorityLatency[i] / priorityCount[i]) << "ms" << "\n  最大延迟: " << maxLatency[i] << "ms" << std::endl;
        }
    }

    pool.Shutdown();
}

/// <summary>
/// 生成随机日志消息
/// </summary>
/// <param name="length">消息长度</param>
/// <returns>随机消息字符串</returns>
std::string GenerateRandomMessage(size_t length)
{
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, sizeof(charset) - 2);

    std::string str(length, 0);
    for (size_t i = 0; i < length; ++i)
    {
        str[i] = charset[dis(gen)];
    }
    return str;
}

/// <summary>
/// 日志系统压力测试类
/// </summary>
class LogSystemStressTest
{
public:
    /// <summary>
    /// 构造函数
    /// </summary>
    /// <param name="threadCount">测试线程数</param>
    /// <param name="messageCount">每个线程的消息数</param>
    /// <param name="messageSize">消息大小</param>
    LogSystemStressTest(size_t threadCount, size_t messageCount, size_t messageSize)
        : m_threadCount(threadCount), m_messageCount(messageCount), m_messageSize(messageSize)
    {
        // 配置线程池
        ST_ThreadPoolConfig threadConfig;
        threadConfig.m_minThreads = threadCount;
        threadConfig.m_maxThreads = threadCount;
        threadConfig.m_maxQueueSize = messageCount * threadCount;
        threadConfig.m_keepAliveTime = 1000;
        m_threadPool = std::make_unique<ThreadPool>(threadConfig);

        // 初始化日志系统
        ST_LogConfig logConfig;
        logConfig.m_logFilePath = "stress_test.log";
        logConfig.m_logLevel = EM_LogLevel::Info;
        logConfig.m_maxFileSize = 1024 * 1024 * 1024; // 1GB
        logConfig.m_asyncEnabled = true;
        LogSystem::Instance().Initialize(logConfig);
    }

    /// <summary>
    /// 运行压力测试
    /// </summary>
    void RunTest()
    {
        std::cout << "Starting stress test with:" << std::endl << "Thread count: " << m_threadCount << std::endl << "Messages per thread: " << m_messageCount << std::endl << "Message size: " << m_messageSize << " bytes" << std::endl;

        auto startTime = std::chrono::high_resolution_clock::now();
        std::vector<std::future<void>> futures;

        // 提交测试任务
        for (size_t i = 0; i < m_threadCount; ++i)
        {
            futures.push_back(m_threadPool->Submit([this, i]()
            {
                this->WriteLogsWorker(i);
            }));
        }

        // 等待所有任务完成
        for (auto& future : futures)
        {
            future.wait();
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        // 确保所有日志都被写入
        LogSystem::Instance().Flush();

        // 计算并显示统计信息
        size_t totalMessages = m_threadCount * m_messageCount;
        double totalSeconds = duration.count() / 1000.0;
        double messagesPerSecond = totalMessages / totalSeconds;
        double mbPerSecond = (totalMessages * m_messageSize) / (1024.0 * 1024.0 * totalSeconds);

        std::cout << "\nTest completed:" << std::endl << "Total time: " << totalSeconds << " seconds" << std::endl << "Messages per second: " << messagesPerSecond << std::endl << "Throughput: " << mbPerSecond << " MB/s" << std::endl;
    }

private:
    /// <summary>
    /// 工作线程函数
    /// </summary>
    /// <param name="threadId">线程ID</param>
    void WriteLogsWorker(size_t threadId)
    {
        std::vector<EM_LogLevel> levels = {EM_LogLevel::Debug, EM_LogLevel::Info, EM_LogLevel::Warning, EM_LogLevel::Error, EM_LogLevel::Fatal};

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> levelDis(0, levels.size() - 1);

        for (size_t i = 0; i < m_messageCount; ++i)
        {
            std::string message = GenerateRandomMessage(m_messageSize);
            EM_LogLevel level = levels[levelDis(gen)];
            LogSystem::Instance().WriteLog(level, QString::fromStdString("Thread-" + std::to_string(threadId) + ": " + message));
        }
    }

private:
    size_t m_threadCount;                     ///< 测试线程数
    size_t m_messageCount;                    ///< 每个线程的消息数
    size_t m_messageSize;                     ///< 消息大小
    std::unique_ptr<ThreadPool> m_threadPool; ///< 线程池
};

/// <summary>
/// 主函数
/// </summary>
int main()
{
    try
    {
        // 创建压力测试实例
        // 参数：8个线程，每个线程100000条消息，每条消息128字节
        LogSystemStressTest test(8, 100000, 128);
        test.RunTest();
    } catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
    try
    {
        std::cout << "=== 开始线程池性能测试 ===" << std::endl;

        // 执行计算密集型任务测试
        TestComputeBoundTasks();

        // 等待一段时间
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // 执行IO密集型任务测试
        TestIOBoundTasks();

        // 等待一段时间
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // 执行混合优先级任务测试
        TestMixedPriorityTasks();

        std::cout << "\n所有测试完成!" << std::endl;
    } catch (const std::exception& e)
    {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
