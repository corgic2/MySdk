/// <summary>
/// 主程序入口
/// </summary>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "LogSystem/LogSystem.h"
#include "ThreadPool/ThreadPool.h"

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

    const int messageCount = 10000;
    std::vector<std::thread> threads;
    for (int t = 0; t < 4; ++t)
    {
        threads.emplace_back([t, messageCount]()
        {
            for (int i = t * (messageCount/4); i < (t + 1) * (messageCount/4); ++i)
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
        }
        catch (const std::exception& e)
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
/// 主函数
/// </summary>
int main()
{
    try
    {
        std::cout << "=== 开始测试 ===" << std::endl;
        
        TestLogSystem();
        TestThreadPool();

        std::cout << "\n所有测试完成!" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}