/**
 * @file scheduler.cpp
 * @brief 协程调度器实现
 */

#include "scheduler.h"
#include "marco.h"
#include "hook.h"
namespace sylar
{
    static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

    // 当前线程的调度器，同一个调度器下的所有线程共享同一个实例
    static thread_local Scheduler *t_scheduler = nullptr;

    // 当前线程的调度协程，每个线程都独有一份
    static thread_local Fiber *t_scheduler_fiber = nullptr;

    Scheduler::Scheduler(size_t threads, bool use_caller, const std::string &name)
    {
        SYLAR_ASSERT(threads > 0);

        m_useCaller = use_caller;

        m_name = name;

        if (use_caller)
        {
            --threads;
            sylar::Fiber::GetThis();
            SYLAR_ASSERT(GetThis() == nullptr);
            t_scheduler = this;

            /**
             * caller线程的主协程不会被线程的调度协程run进行调度，并且当线程的调度协程停止时，应该返回caller线程的主协程
             * 在use caller情况下，把caller线程的主协程暂时保存起来，等调度协程结束时，再resume caller协程
             */
            m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, false));

            sylar::Thread::SetName(m_name);
            t_scheduler_fiber = m_rootFiber.get();
            m_rootThread = sylar::GetThreadId();
            m_threadIds.push_back(m_rootThread);
        }
        else
        {
            m_rootThread = -1;
        }
        m_threadCount = threads;
    }

    Scheduler *Scheduler::GetThis()
    {
        return t_scheduler;
    }

    Fiber *Scheduler::GetMainFiber()
    {
        return t_scheduler_fiber;
    }

    void Scheduler::setThis()
    {
        t_scheduler = this;
    }

    Scheduler::~Scheduler()
    {
        SYLAR_LOG_DEBUG(g_logger) << "Scheduler::~Schedulr()";
        SYLAR_ASSERT(m_stopping);
        if (GetThis() == this)
        {
            t_scheduler = nullptr;
        }
    }

    void Scheduler::start()
    {
        SYLAR_LOG_DEBUG(g_logger) << "start";
        MutexType::Lock lock(m_mutex);
        if (m_stopping)
        {
            SYLAR_LOG_ERROR(g_logger) << "Scheduler is stopped";
            return;
        }
        SYLAR_ASSERT(m_threads.empty());
        m_threads.resize(m_threadCount);

        for (size_t i = 0; i < m_threadCount; i++)
        {
            m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this),
                                          m_name + "_" + std::to_string(i)));
            m_threadIds.push_back(m_threads[i]->getId());
        }
    }

    bool Scheduler::stopping()
    {
        MutexType::Lock lock(m_mutex);
        return m_stopping && m_tasks.empty() && m_activeThreadCount == 0;
    }

    void Scheduler::tickle()
    {
        SYLAR_LOG_DEBUG(g_logger) << "tickle";
    }

    void Scheduler::idle()
    {
        SYLAR_LOG_DEBUG(g_logger) << "idle";
        while (!stopping())
            [sylar::Fiber::GetThis()->yield();
            ]
    }

    void Scheduler::stop()
    {
        SYLAR_LOG_DEBUG(g_logger) << "stop";
        if (stopping())
            return;
        m_stopping = true;

        if (m_useCaller)
        {
            SYLAR_ASSERT(GetThis() == this);
        }
        else
        {
            SYLAR_ASSERT(GetThis != this);
        }

        for (size_t i = 0; i < m_threadCount; i++)
        {
            tickle();
        }

        if (m_rootFiber)
        {
            tickle();
        }

        // 在use caller情况下，调度器协程结束时，应该返回caller协程
        if (m_rootFiber)
        {
            m_rootFiber->resume();
            SYLAR_LOG_DEBUG(g_logger) << "m_rootFiber end";
        }

        std::vector<Thread::ptr> thrs;
        {
            MutexType::Lock lock(m_mutex);
            thrs.swap(m_threads);
        }

        for (auto &i : thrs)
        {
            i->join();
        }
    }

    void Scheduler::run()
    {
        SYLAR_LOG_DEBUG(g_logger) << "run";
        set_hook_enable(true);
        setThis();
        if (sylar::GetThreadId() != m_rootThread)
        {
            t_scheduler_fiber = sylar::Fiber::GetThis().get();
        }

        Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
        Fiber::ptr cb_fiber;

        SchedulerTask task;
        while (true)
        {
            task.reset();
            bool tickle_me = false;
            {
                MutexType::Lock lock(m_mutex);
                auto it = m_tasks.begin();
                while (it != m_tasks.end())
                {
                    if (it->thread != -1 && it->thread != sylar::GetThreadId())
                    {
                        ++it;
                        tickle_me = true;
                        continue;
                    }
                }

                // 找到一个未指定线程，或是指定了当前线程的任务
                SYLAR_ASSERT(it->fiebr || it->cb);

                if (it->fiber && it->fiber->getState() == Fiber::RUNNING)
                {
                    ++it;
                    continue;
                }

                // 当前调度线程找到一个任务，准备开始调度，将其从任务队列中剔除，活动线程数加1
                task = *it;
                m_tasks.erase(it++);
                ++m_activeThreadCount;
                break;
            }
            // 当前线程拿完一个任务后，发现任务队列还有剩余，那么tickle一下其他线程
            tickle_me |= (it != m_tasks.end());
        }
        if (tickle_me)
        {
            tickle();
        }

        if (task.fiber)
        {
            // resume协程，resume返回时，协程要么执行完了，要么半路yield了，总之这个任务就算完成了，活跃线程数减一
            task.fiber->resume();
            --m_activeThreadCount;
            task.reset();
        }
        else if (task.cb)
        {
            if (cb_fiber)
            {
                cb_fiber->reset(task.cb);
            }
            else
            {
                cb_fiber.reset(new Fiber(task.cb));
            }
            task.reset();
            cb_fiber->resume();
            --m_activeThreadCount;
            cb_fiber.reset();
        }
        else
        {
            // 进到这个分支情况一定是任务队列空了，调度idle协程即可
            if (idle_fiber->getState() == Fiber::TERM)
            {
                // 如果调度器没有调度任务，那么idle协程会不停地resume/yield，不会结束，如果idle协程结束了，那一定是调度器停止了
                SYLAR_LOG_DEBUG(g_logger) << "idle fiber term";
                break;
            }
            ++m_idleThreadCount;
            idle_fiber->resume();
            --m_idleThreadCount;
        }
    }
    SYLAR_LOG_DEBUG(g_logger) << "Scheduler::run() exit";
} // namespace sylar
