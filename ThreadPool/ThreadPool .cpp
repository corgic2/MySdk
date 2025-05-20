#include <random>
#include "ThreadPool.h"
thread_local int ThreadPool::m_threadIndex = 0;

void ThreadPool::WorkThread()
{
    while (!m_globalDone.load())
    {
        bool bflag = false;
        // 先从自己的队列中取任务
        if (WorkThread(m_threadIndex))
        {
            bflag = true;
        }
        // 未取到任务时，再从其他队列中取任务
        if (!bflag)
        {
            int stealIndex = (m_threadIndex + 1) % m_localWorkQueue.size();
            for (int i = stealIndex; i < m_localWorkQueue.size();)
            {
                if (i == m_threadIndex)
                {
                    break; // 不偷自己的队列
                }
                if (TrySteal(i))
                {
                    bflag = true;
                    break;
                }
                i = (i + 1) % m_localWorkQueue.size(); // 轮询下一个队列
            }
        }
        // 如果都没有任务，则从全局队列中取任务
        if (!bflag)
        {
            std::unique_lock<std::mutex> lock(m_globalMutex);
            m_condition.wait_for(lock, std::chrono::milliseconds(100), [this]()
            {
                return !m_workGlobalQueue.empty() || m_globalDone.load();
            });
            WorkGlobalThread();
        }
    }
}

int ThreadPool::FindLightestQueue()
{
    // 随机采样3个队列选择最轻负载
    constexpr size_t SAMPLE_SIZE = 3;
    std::vector<size_t> candidates(SAMPLE_SIZE);

    static thread_local std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, m_localWorkQueue.size() - 1);

    size_t min_size = std::numeric_limits<size_t>::max();
    size_t target_idx = 0;

    for (size_t i = 0; i < SAMPLE_SIZE; ++i)
    {
        size_t idx = dist(gen);
        std::lock_guard<std::mutex> lock(m_localWorkQueue[idx].mutex);
        if (m_localWorkQueue[idx].queue->size() < min_size)
        {
            min_size = m_localWorkQueue[idx].queue->size();
            target_idx = idx;
        }
    }
    return target_idx;
}

ThreadPool::ThreadPool()
{
    m_globalDone = false;
    int threadCount = std::thread::hardware_concurrency();

    for (int i = 0; i < threadCount; ++i)
    {
        m_threads.emplace_back([this, i]()
        {
            m_threadIndex = i;
            WorkThread();
        });
        ST_localThreadQueue localQueue;
        localQueue.queue = std::make_unique<m_threadQueue>();
        m_localWorkQueue.emplace_back(std::move(localQueue));
    }
}

ThreadPool::~ThreadPool()
{
    m_globalDone.store(true);
    for (auto& thread : m_threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
    m_condition.notify_all();
}

bool ThreadPool::TrySteal(size_t index)
{
    if (m_localWorkQueue[index].mutex.try_lock())
    {
        std::lock_guard<std::mutex> guard(m_localWorkQueue[index].mutex, std::adopt_lock);
        if (m_localWorkQueue[index].queue && !m_localWorkQueue[index].queue->empty())
        {
            auto task = std::move(m_localWorkQueue[index].queue->front());
            m_localWorkQueue[index].queue->pop();
            task();
            return true;
        }
    }
    return false;
}

bool ThreadPool::WorkThread(size_t index)
{
    std::lock_guard<std::mutex> localMutex(m_localWorkQueue[index].mutex);
    if (m_localWorkQueue[index].queue && !m_localWorkQueue[index].queue->empty())
    {
        auto work = std::move(m_localWorkQueue[index].queue->front());
        m_localWorkQueue[index].queue->pop();
        work();
        return true;
    }
    return false;
}

bool ThreadPool::WorkGlobalThread()
{
    if (!m_workGlobalQueue.empty())
    {
        auto work = std::move(m_workGlobalQueue.front());
        m_workGlobalQueue.pop();
        work();
        return true;
    }
    return false;
}
