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
#include <shared_mutex>
#include <array>

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
        if (m_priority != other.m_priority) {
            return m_priority < other.m_priority;
        }
        return m_submitTime > other.m_submitTime;  // 相同优先级时，先进先出
    }
};

/// <summary>
/// 无锁任务队列实现
/// </summary>
class LockFreeTaskQueue {
private:
    static constexpr size_t QUEUE_SIZE = 10000;
    std::array<ST_Task, QUEUE_SIZE> m_buffer;
    std::atomic<size_t> m_head{0};
    std::atomic<size_t> m_tail{0};

public:
    /// <summary>
    /// 尝试将任务推入队列
    /// </summary>
    bool try_push(const ST_Task& task) {
        size_t tail = m_tail.load(std::memory_order_relaxed);
        size_t next_tail = (tail + 1) % QUEUE_SIZE;
        
        if (next_tail == m_head.load(std::memory_order_acquire)) {
            return false; // 队列已满
        }
        
        m_buffer[tail] = task;
        m_tail.store(next_tail, std::memory_order_release);
        return true;
    }

    /// <summary>
    /// 尝试从队列中取出任务
    /// </summary>
    bool try_pop(ST_Task& task) {
        size_t head = m_head.load(std::memory_order_relaxed);
        
        if (head == m_tail.load(std::memory_order_acquire)) {
            return false; // 队列为空
        }
        
        task = m_buffer[head];
        m_head.store((head + 1) % QUEUE_SIZE, std::memory_order_release);
        return true;
    }

    /// <summary>
    /// 获取队列大小
    /// </summary>
    size_t size() const {
        size_t head = m_head.load(std::memory_order_relaxed);
        size_t tail = m_tail.load(std::memory_order_relaxed);
        if (tail >= head) {
            return tail - head;
        }
        return QUEUE_SIZE - (head - tail);
    }

    /// <summary>
    /// 检查队列是否为空
    /// </summary>
    bool empty() const {
        return size() == 0;
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
        
        if (m_stop)
        {
            throw std::runtime_error("ThreadPool is stopped");
        }

        ST_Task taskWrapper;
        taskWrapper.m_func = [task]() { (*task)(); };
        taskWrapper.m_priority = priority;
        taskWrapper.m_submitTime = std::chrono::steady_clock::now();

        if (!m_tasks.try_push(std::move(taskWrapper)))
        {
            throw std::runtime_error("Task queue is full");
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
    size_t GetTaskCount() { return m_tasks.size(); }

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
    LockFreeTaskQueue m_tasks; ///< 无锁任务队列
    mutable std::shared_mutex m_configMutex; ///< 配置互斥锁
    mutable std::mutex m_workersMutex; ///< 工作线程集合互斥锁
    std::condition_variable_any m_condition; ///< 条件变量
    ST_ThreadPoolConfig m_config; ///< 线程池配置
    std::atomic<bool> m_stop{false}; ///< 停止标志
    std::atomic<size_t> m_totalThreads{0}; ///< 总线程数
    std::atomic<size_t> m_activeThreads{0}; ///< 活动线程数
    std::atomic<bool> m_adjusting{false}; ///< 线程池调整标志
};
