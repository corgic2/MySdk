#include <condition_variable>
#include <future>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include "../SDKCommonDefine/SDK_Export.h"
#pragma once

class FuncTemplateClass
{
public:
    struct ST_FuncBase
    {
        virtual void Call() = 0;
        virtual ~ST_FuncBase() = default;
    };

    template <typename Func>
    struct ST_FuncImpl : public ST_FuncBase
    {
        void Call() override
        {
            m_func();
        }

        ST_FuncImpl(Func&& func)
            : m_func(std::move(func))
        {
        }

        Func m_func;
    };

    FuncTemplateClass() = default;
    //三种拷贝构造删除
    FuncTemplateClass(const FuncTemplateClass&) = delete;
    FuncTemplateClass& operator=(const FuncTemplateClass&) = delete;
    FuncTemplateClass(FuncTemplateClass&) = delete;

    template <typename Func>
    FuncTemplateClass(Func&& funcObject)
        : m_impl(new ST_FuncImpl<Func>(std::forward<Func>(funcObject)))
    {
    }

    FuncTemplateClass(FuncTemplateClass&& object)
    {
        m_impl = std::move(object.m_impl);
        object.m_impl = nullptr;
    }

    FuncTemplateClass& operator=(FuncTemplateClass&& object)
    {
        if (this != &object)
        {
            m_impl = std::move(object.m_impl);
            object.m_impl = nullptr;
        }
        return *this;
    }

    void operator()()
    {
        if (m_impl)
        {
            m_impl->Call();
        }
    }

private:
    std::unique_ptr<ST_FuncBase> m_impl = nullptr;
};


class ThreadPool
{
public:
    ThreadPool();
    ~ThreadPool();

    template <typename Func>
    std::future<std::invoke_result_t<Func>> AddTask(Func f)
    {
        using result = std::invoke_result_t<Func>;
        std::packaged_task<result()> task(std::move(f));
        // task不可复制，只能通过移动,因此，task内的函数也应当仅支持移动
        // 并且要获取函数返回值，则同步需要模板函数编程，因此自定义函数类
        std::future<result> res = task.get_future();
        // 每次寻找最轻松的队列，以防任务不均衡
        size_t idx = FindLightestQueue();
        std::unique_lock<std::mutex> lock(m_localWorkQueue[idx].mutex, std::try_to_lock);
        if (lock.owns_lock() && m_localWorkQueue[idx].queue)
        {
            m_localWorkQueue[idx].queue->emplace(std::move(task));
            return res;
        }
        // 回退到全局队列
        std::lock_guard<std::mutex> globalLock(m_globalMutex);
        m_workGlobalQueue.emplace(std::move(task));
        m_condition.notify_one();
        return res;
    }

private:
    using m_threadQueue = std::queue<FuncTemplateClass>;

    struct ST_localThreadQueue
    {
        ST_localThreadQueue() = default;

        ST_localThreadQueue(ST_localThreadQueue&& other)
            : queue(std::move(other.queue))
        {
        }

        ST_localThreadQueue& operator=(ST_localThreadQueue&& other)
        {
            if (this != &other)
            {
                queue = std::move(other.queue);
            }
            return *this;
        }

        ST_localThreadQueue& operator=(const ST_localThreadQueue& other) = delete;
        ST_localThreadQueue(const ST_localThreadQueue& other) = delete;
        ST_localThreadQueue(ST_localThreadQueue& other) = delete;

        std::unique_ptr<m_threadQueue> queue;
        std::mutex mutex;
    };

    void WorkThread();
    int FindLightestQueue();
    bool TrySteal(size_t index);
    bool WorkThread(size_t index);
    bool WorkGlobalThread();
private:
    std::atomic_bool m_globalDone;
    std::queue<FuncTemplateClass> m_workGlobalQueue;
    std::mutex m_globalMutex;
     
    std::vector<ST_localThreadQueue> m_localWorkQueue; // 根据当前线程不同各自拥有一个任务队列，减少全局队列的锁竞争
    static thread_local int m_threadIndex;

    std::atomic<size_t> m_nextQueueIndex{0}; // 新增原子计数器
    std::vector<std::thread> m_threads;

    std::condition_variable m_condition;
};

class SDK_API GlobalThreadPool
{
public:
    GlobalThreadPool()
    {
        m_pool = std::make_unique<ThreadPool>();
    }

    ~GlobalThreadPool()
    {
    }

private:
    std::unique_ptr<ThreadPool> m_pool = nullptr;
};
