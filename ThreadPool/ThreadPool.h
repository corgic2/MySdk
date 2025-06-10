/// <summary>
/// 线程池头文件
/// </summary>
#pragma once
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <atomic>
#include <type_traits>
#include "../SDKCommonDefine/SDK_Export.h"

/// <summary>
/// 线程池任务优先级枚举
/// </summary>
enum class EM_TaskPriority
{
    Low,        ///< 低优先级
    Normal,     ///< 普通优先级
    High,       ///< 高优先级
    Critical    ///< 关键优先级
};

/// <summary>
/// 线程池配置结构体
/// </summary>
struct ST_ThreadPoolConfig
{
    size_t m_minThreads; ///< 最小线程数
    size_t m_maxThreads; ///< 最大线程数
    size_t m_maxQueueSize; ///< 最大队列大小
    size_t m_keepAliveTime; ///< 空闲线程保持时间(毫秒)

    /// <summary>
    /// 构造函数，初始化默认配置
    /// </summary>
    ST_ThreadPoolConfig()
        : m_minThreads(2)
        , m_maxThreads(std::thread::hardware_concurrency())
        , m_maxQueueSize(10000)
        , m_keepAliveTime(60000) // 1分钟
    {
    }
};

/// <summary>
/// 线程池任务结构体
/// </summary>
struct ST_Task
{
    std::function<void()> m_func; ///< 任务函数
    EM_TaskPriority m_priority; ///< 任务优先级
    std::chrono::steady_clock::time_point m_submitTime;  ///< 提交时间

    /// <summary>
    /// 任务比较函数，用于优先级队列
    /// </summary>
    /// <param name="other">另一个任务对象</param>
    /// <returns>当前任务优先级是否小于另一个任务</returns>
    bool operator<(const ST_Task& other) const
    {
        return m_priority < other.m_priority;
    }
};

/// <summary>
/// 线程池类，提供基于优先级的任务调度功能
/// </summary>
class SDK_API ThreadPool
{
public:
    /// <summary>
    /// 构造函数
    /// </summary>
    /// <param name="config">线程池配置</param>
    explicit ThreadPool(const ST_ThreadPoolConfig& config = ST_ThreadPoolConfig());

    /// <summary>
    /// 析构函数
    /// </summary>
    ~ThreadPool();

    /// <summary>
    /// 提交任务到线程池
    /// </summary>
    /// <param name="task">任务函数</param>
    /// <param name="priority">任务优先级</param>
    /// <returns>future对象，用于获取任务结果</returns>
    template <typename F>
    auto Submit(F&& f, EM_TaskPriority priority = EM_TaskPriority::Normal) 
        -> std::future<decltype(std::declval<std::decay_t<F>>()())>
    {
        using return_type = decltype(std::declval<std::decay_t<F>>()());

        auto task = std::make_shared<std::packaged_task<return_type()>>(std::forward<F>(f));
        std::future<return_type> res = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(m_mutex);

            if (m_stop)
            {
                throw std::runtime_error("ThreadPool is stopped");
            }

            if (m_tasks.size() >= m_config.m_maxQueueSize)
            {
                throw std::runtime_error("Task queue is full");
            }

            ST_Task taskWrapper;
            taskWrapper.m_func = [task]() { (*task)(); };
            taskWrapper.m_priority = priority;
            taskWrapper.m_submitTime = std::chrono::steady_clock::now();

            m_tasks.push(std::move(taskWrapper));
        }

        m_condition.notify_one();
        return res;
    }

    /// <summary>
    /// 获取当前线程数
    /// </summary>
    /// <returns>当前线程数</returns>
    size_t GetCurrentThreadCount() const { return m_totalThreads.load(); }

    /// <summary>
    /// 获取当前任务数
    /// </summary>
    /// <returns>当前任务数</returns>
    size_t GetTaskCount() 
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_tasks.size();
    }

    /// <summary>
    /// 调整线程池大小
    /// </summary>
    /// <param name="minThreads">最小线程数</param>
    /// <param name="maxThreads">最大线程数</param>
    void Resize(size_t minThreads, size_t maxThreads);

    /// <summary>
    /// 等待所有任务完成
    /// </summary>
    void WaitAll();

    /// <summary>
    /// 停止线程池
    /// </summary>
    void Shutdown();

private:
    /// <summary>
    /// 工作线程函数
    /// </summary>
    void WorkerThread();

    /// <summary>
    /// 调整工作线程数量
    /// </summary>
    void AdjustThreadCount();

    /// <summary>
    /// 创建新的工作线程
    /// </summary>
    void CreateWorkerThread();

private:
    std::vector<std::thread> m_workers; ///< 工作线程集合
    std::priority_queue<ST_Task> m_tasks; ///< 任务队列
    mutable std::mutex m_mutex; ///< 互斥锁
    std::condition_variable m_condition; ///< 条件变量
    ST_ThreadPoolConfig m_config; ///< 线程池配置
    std::atomic<bool> m_stop; ///< 停止标志
    std::atomic<size_t> m_totalThreads; ///< 总线程数
};
