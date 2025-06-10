#include "ThreadPool.h"
#include <algorithm>

ThreadPool::ThreadPool(const ST_ThreadPoolConfig& config)
    : m_config(config)
    , m_stop(false)
    , m_totalThreads(0)
    , m_activeThreads(0)
    , m_adjusting(false)
{
    for (size_t i = 0; i < m_config.m_minThreads; ++i)
    {
        CreateWorkerThread();
    }
}

ThreadPool::~ThreadPool()
{
    if (!m_stop)
    {
        try 
        {
            Shutdown();
        }
        catch (...) {}
    }
}

void ThreadPool::Resize(size_t minThreads, size_t maxThreads)
{
    if (minThreads > maxThreads)
    {
        throw std::invalid_argument("minThreads cannot be greater than maxThreads");
    }

    bool expected = false;
    if (!m_adjusting.compare_exchange_strong(expected, true))
    {
        return;
    }

    {
        std::unique_lock<std::shared_mutex> lock(m_configMutex);
        m_config.m_minThreads = minThreads;
        m_config.m_maxThreads = maxThreads;
    }

    AdjustThreadCount();
    m_adjusting.store(false);
}

void ThreadPool::WaitAll()
{
    const auto timeout = std::chrono::milliseconds(100);
    while (true)
    {
        if (m_tasks.empty() && (m_activeThreads.load() == 0) || m_stop)
        {
            break;
        }
        
        std::this_thread::sleep_for(timeout);
    }
}

void ThreadPool::Shutdown()
{
    // 设置停止标志，不需要锁，因为是原子操作
    bool expected = false;
    if (!m_stop.compare_exchange_strong(expected, true))
    {
        return;  // 已经在关闭过程中
    }

    // 清理任务队列
    ST_Task task;
    while (m_tasks.try_pop(task)) {} // 清空队列

    // 通知所有等待的线程
    m_condition.notify_all();

    // 逐个清理线程
    std::vector<std::thread> workers_to_join;
    {
        std::lock_guard<std::mutex> lock(m_workersMutex);
        workers_to_join.swap(m_workers);
    }

    // 在没有持有锁的情况下等待线程结束
    for (auto& worker : workers_to_join)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }

    // 重置状态
    m_totalThreads = 0;
    m_activeThreads = 0;
    m_adjusting = false;
}

void ThreadPool::WorkerThread()
{
    while (true)
    {
        ST_Task task;
        bool hasTask = false;

        // 使用较短的超时时间等待任务
        {
            std::shared_lock<std::shared_mutex> configLock(m_configMutex);
            auto timeout = std::chrono::milliseconds(m_config.m_keepAliveTime);
            configLock.unlock();

            auto startWait = std::chrono::steady_clock::now();
            while (!m_stop)
            {
                if (m_tasks.try_pop(task))
                {
                    hasTask = true;
                    break;
                }

                if (std::chrono::steady_clock::now() - startWait > timeout)
                {
                    break;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        // 检查停止信号
        if (m_stop)
        {
            --m_totalThreads;
            return;
        }

        // 检查空闲超时
        if (!hasTask)
        {
            std::shared_lock<std::shared_mutex> configLock(m_configMutex);
            if (m_totalThreads > m_config.m_minThreads)
            {
                --m_totalThreads;
                return;
            }
            continue;
        }

        // 执行任务
        try
        {
            ++m_activeThreads;
            task.m_func();
        }
        catch (...) {}
        --m_activeThreads;

        // 只在非停止状态下调整线程数
        if (!m_stop && !m_adjusting)
        {
            AdjustThreadCount();
        }
    }
}

void ThreadPool::AdjustThreadCount()
{
    std::shared_lock<std::shared_mutex> configLock(m_configMutex);
    size_t currentThreads = m_totalThreads;
    size_t pendingTasks = m_tasks.size();
    size_t activeThreads = m_activeThreads;
    size_t minThreads = m_config.m_minThreads;
    size_t maxThreads = m_config.m_maxThreads;
    configLock.unlock();

    // 增加线程
    if (pendingTasks > activeThreads && currentThreads < maxThreads)
    {
        size_t threadsToAdd = std::min(
            maxThreads - currentThreads,
            pendingTasks - activeThreads
        );

        for (size_t i = 0; i < threadsToAdd; ++i)
        {
            CreateWorkerThread();
        }
    }
    // 减少线程
    else if (currentThreads > minThreads && activeThreads < currentThreads / 2)
    {
        // 通过条件变量通知来自然减少线程数
        m_condition.notify_all();
    }
}

void ThreadPool::CreateWorkerThread()
{
    std::lock_guard<std::mutex> lock(m_workersMutex);
    ++m_totalThreads;
    m_workers.emplace_back(&ThreadPool::WorkerThread, this);
}
