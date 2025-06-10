#include "ThreadPool.h"
#include <algorithm>

ThreadPool::ThreadPool(const ST_ThreadPoolConfig& config)
    : m_config(config)
    , m_stop(false)
    , m_totalThreads(0)
{
    for (size_t i = 0; i < m_config.m_minThreads; ++i)
    {
        CreateWorkerThread();
    }
}

ThreadPool::~ThreadPool()
{
    Shutdown();
}

void ThreadPool::Resize(size_t minThreads, size_t maxThreads)
{
    if (minThreads > maxThreads)
    {
        throw std::invalid_argument("minThreads cannot be greater than maxThreads");
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    m_config.m_minThreads = minThreads;
    m_config.m_maxThreads = maxThreads;

    AdjustThreadCount();
}

void ThreadPool::WaitAll()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_condition.wait(lock, [this]()
    {
        return m_tasks.empty() && (m_totalThreads.load() == 0);
    });
}

void ThreadPool::Shutdown()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stop = true;
    }

    m_condition.notify_all();

    for (std::thread& worker : m_workers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }

    m_workers.clear();
    m_totalThreads = 0;
}

void ThreadPool::WorkerThread()
{
    while (true)
    {
        ST_Task task;
        {
            std::unique_lock<std::mutex> lock(m_mutex);

            m_condition.wait(lock, [this]()
            {
                return m_stop || !m_tasks.empty();
            });

            if (m_stop && m_tasks.empty())
            {
                --m_totalThreads;
                return;
            }

            if (!m_tasks.empty())
            {
                task = std::move(const_cast<ST_Task&>(m_tasks.top()));
                m_tasks.pop();
            }
        }

        if (task.m_func)
        {
            task.m_func();
            AdjustThreadCount();
        }
    }
}

void ThreadPool::AdjustThreadCount()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    size_t currentThreads = m_totalThreads;
    size_t pendingTasks = m_tasks.size();

    // 如果任务队列增长，且当前线程数小于最大线程数，则创建新线程
    if (pendingTasks > currentThreads && currentThreads < m_config.m_maxThreads)
    {
        size_t threadsToAdd = std::min(
                                       m_config.m_maxThreads - currentThreads,
                                       pendingTasks - currentThreads
                                      );

        for (size_t i = 0; i < threadsToAdd; ++i)
        {
            CreateWorkerThread();
        }
    }
    // 如果空闲线程太多，且当前线程数大于最小线程数，则减少线程
    else if (currentThreads > m_config.m_minThreads)
    {
        // 通过设置stop标志来让多余的线程自然退出
        size_t threadsToRemove = currentThreads - std::max(
                                                           m_config.m_minThreads,
                                                           currentThreads / 2
                                                          );

        if (threadsToRemove > 0)
        {
            m_stop = true;
            m_condition.notify_all();
            m_stop = false;
        }
    }
}

void ThreadPool::CreateWorkerThread()
{
    ++m_totalThreads;
    m_workers.emplace_back(&ThreadPool::WorkerThread, this);
}
