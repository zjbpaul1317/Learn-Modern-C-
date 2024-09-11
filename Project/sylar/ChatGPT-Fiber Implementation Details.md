## Prompt:
void Fiber::MainFunc() { Fiber::ptr cur = GetThis(); // GetThis()的shared\_from\_this()方法让引用计数加1 SYLAR\_ASSERT(cur); cur->m\_cb(); cur->m\_cb = nullptr; cur->m\_state = TERM; auto raw\_ptr = cur.get(); // 手动让t\_fiber的引用计数减1 cur.reset(); raw\_ptr->yield(); } 详细讲述上面代码，请为每行代码加上注释

## Response:
```cpp
cppvoid Fiber::MainFunc() {
    Fiber::ptr cur = GetThis(); // 获取当前协程对象的shared_ptr，并将其存入cur。GetThis()调用shared_from_this()方法，让当前协程对象的引用计数加1
    SYLAR_ASSERT(cur); // 断言当前
```

## Prompt:
/\*\* \* @file fiber.cpp \* @brief 协程实现 \*/ #include <atomic> #include "../fiber.h" #include "config.h" #include "log.h" #include "macro.h" #include "scheduler.h" namespace sylar { static Logger::ptr g\_logger = SYLAR\_LOG\_NAME("system"); //全局静态变量，用于生成协程id static std::atomic<uint64\_t> s\_fiber\_id{0}; //全局静态变量，用于统计当前的协程数 static std::atomic<uint64\_t> s\_fiber\_count{0}; //线程局部变量，当前线程正在运行的协程 static thread\_local Fiber\* t\_fiber = nullptr; //线程局部变量，当前线程的主协程，切换到这个协程，就相当于切换到了主线程中运行，智能指针形式 static thread\_local Fiber::ptr t\_thread\_fiber = nullptr; //协程栈大小，可通过配置文件获取，默认128k static ConfigVar<uint32\_t>::ptr g\_fiber\_stack\_size = Config::Lookup<uint32\_t>("fiber.stack\_size", 128\*1024, "fiber stack size"); /\*\* \* @brief malloc栈内存分配器 \*/ class MallocStackAllocator { public: static void \*Alloc(size\_t size) { return malloc(size); } static void Dealloc(void\* vp, size\_t size) { return free(vp); } }; using StackAllocator = MallocStackAllocator; uint64\_t Fiber::GetFiberId() { if(t\_fiber) { return t\_fiber->getId(); } return 0; } Fiber::Fiber() { SetThis(this); m\_state = RUNNING; if(getcontext(&m\_ctx)) { SYLAR\_ASSERT2(false, "getcontext"); } ++s\_fiber\_count; m\_id = s\_fiber\_id++; //协程id从0开始，用完加1 SYLAR\_LOG\_DEBUG(g\_logger) << "Fiber::Fiber() main id = " << m\_id; } void Fiber::SetThis(Fiber \*f) { t\_fiber = f; } /\*\* \* @brief 获取当前协程，同时充当初始化当前线程主协程的作用，此函数在使用协程之前需要调用 \*/ Fiber::ptr Fiber::GetThis() { if(t\_fiber) { return t\_fiber->shared\_from\_this(); } Fiber::ptr main\_fiber(new Fiber); SYLAR\_ASSERT(t\_fiber == main\_fiber.get()); t\_thread\_fiber = main\_fiber; return t\_fiber->shared\_from\_this(); } /\*\* \* @brief 带参数的构造函数用于创建其他协程，需要分配栈 \*/ Fiber::Fiber(std::function<void() cb, size\_t stacksize, bool run\_in\_schduler) : m\_id(s\_fiber\_id++) , m\_cb(cb) , m\_runInScheduler(run\_in\_scheduler) { ++s\_fiber\_count; m\_stacksize = stacksize ? stacksize : g\_fiber\_stack\_size->getValue(); m\_stack = StackAllocator::Alloc(m\_stacksize); if(getcontext(&m\_ctx)) { SYLAR\_ASSERT2(false, "getcontext"); } m\_ctx.uc\_link = nullptr; m\_ctx.uc\_stack.ss\_sp = m\_stack; m\_ctx.uc\_stack.ss\_size = m\_stacksize; makecontext(&m\_ctx, &Fiber::MainFunc, 0); SYLAR\_LOG\_DEBUG(g\_logger) << "Fiber::Fiber() id = " << m\_id; } /\*\* \* @brief 线程的主协程析构时需要特殊处理，因为主协程没有分配栈和cb \*/ Fiber::~Fiber() { SYLAR\_LOG\_DEBUG(g\_logger) << "Fiber::~Fiber() id = " <<m\_id; --s\_fiber\_count; if(m\_stack) { //有栈，说明是子协程，需要确保子协程一定是结束状态 SYLAR\_ASSERT(m\_state == TERM); StackAllocator::Dealloc(m\_stack, m\_stacksize); SYLAR\_LOG\_DEBUG(g\_logger) << "dealloc stack, id = " << m\_id; } else {//没有栈，说明是线程的主协程 SYLAR\_ASSERT(!m\_cb); //主协程没有cb SYLAR\_ASSERT(m\_state == RUNNING); //主协程一定是执行状态 Fiber\* cur = t\_fiber; //当前协程就是自己 if(cur == this) { SetThis(nullptr); } } } /\*\* \* @brief 为了简化状态管理，强制只有TERM状态的协程才可以重置，但其实刚创建好但没执行过的协程也应该允许重置的 \*/ void Fiber::reset(std::function<void()> cb) { SYLAR\_ASSERT(m\_stack); SYLAR\_ASSERT(m\_state == TERM); m\_cb = cb; if(getcontext(&m\_ctx)) { SYLAR\_ASSERT2(false, "getcontext"); } m\_ctx.uc\_link = nullptr; m\_ctx.uc\_stack.ss\_sp = m\_stack; m\_ctx.uc\_stack.ss\_size = m\_stacksize; makecontext(&m\_ctx, &Fiber::MainFunc, 0); m\_state = READY; } void Fiber::resume() { SYLAR\_ASSERT(m\_state != TERM && m\_state != RUNNING); SetThis(this); m\_state = RUNNING; //如果协程参与调度器调度，那么应该和调度器的主协程进行swap，而不是线程主协程 if(m\_runInScheduler) { if(swapcontext(&(Scheduler::GetMainFiber()->m\_ctx), &m\_ctx)) { SYLAR\_ASSERT2(false, "swapcontext"); } } else { if(swapcontext(&(t\_thread\_fiber->m\_ctx), &m\_ctx)) { SYLAR\_ASSERT(false, "swapcontext"); } } } void Fiber::yield() { //协程运行完之后会自动yield一次，用于回到主协程，此时状态已为结束状态 SYLAR\_ASSERT(m\_state == RUNNING || m\_state ==TERM); SetThis(t\_thread\_fiber.get()); if(m\_state != TERM) { m\_state = READY; } //如果协程参与调度器调度，那么应该和调度器的主协程进行swap,而不是线程主协程 if(m\_runInScheduler) { if(swapcontext(&m\_ctx, &(Scheduler::GetMainFiber()->m\_ctx))) { SYLAR\_ASSERT2(false, "swapcontext"); } } else { if(swapcontext(&m\_ctx, &(t\_threaed\_fiber->m\_ctx))) { SYLAR\_ASSERT2(false, "swapcontext"); } } } /\*\* \* @brief 这里没有处理协程函数出现异常的情况，同样是为了简化状态管理，并且个人认为协程的异常不应该由框架处理，应该由开发者自行处理 \*/ void Fiber::MainFunc() { Fiber::ptr cur = GetThis(); // GetThis()的shared\_from\_this()方法让引用计数加1 SYLAR\_ASSERT(cur); cur->m\_cb(); cur->m\_cb = nullptr; cur->m\_state = TERM; auto raw\_ptr = cur.get(); // 手动让t\_fiber的引用计数减1 cur.reset(); raw\_ptr->yield(); } } 详细讲述上面代码，请为每行代码加上注释

## Response:
```cpp
cppnamespace sylar
{
    // 初始化全局日志器，用于记录系统日志
    static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

    // 全局静态变量，用于生成协程id，每个协程都会有一个唯一的id
    static std::atomic<uint64_t> s_fiber_id{0};

    // 全局静态变量，用于统计当前存在的协程数
    static std::atomic<uint64_t> s_fiber_count{0};

    // 线程局部变量，指向当前线程正在运行的协程
    static thread_local Fiber* t_fiber = nullptr;

    // 线程局部变量，指向当前线程的主协程，主协程是线程最先创建的协程
    static thread_local Fiber::ptr t_thread_fiber = nullptr;

    // 协程栈大小配置项，默认值为128KB
    static ConfigVar<uint32_t>::ptr g_fiber_stack_size = Config::Lookup<uint32_t>("fiber.stack_size", 128*1024, "fiber stack size");

    // 内存分配器类，用于分配和释放协程的栈内存
    class MallocStackAllocator
    {
    public:
        // 分配指定大小的内存，通常用于协程栈
        static void *Alloc(size_t size) { return malloc(size); }

        // 释放之前分配的内存
        static void Dealloc(void* vp, size_t size) { return free(vp); }
    };

    // 使用上面定义的MallocStackAllocator作为栈分配器
    using StackAllocator  = MallocStackAllocator;

    /**
     * @brief 获取当前协程的id
     * 如果有当前协程，返回它的id；否则返回0
     */
    uint64_t Fiber::GetFiberId()
    {
        if(t_fiber)
        {
            return t_fiber->getId();
        }
        return 0;
    }

    /**
     * @brief Fiber构造函数，用于创建主协程
     * 主协程没有回调函数和栈，只是为了保证线程的协程调度
     */
    Fiber::Fiber()
    {
        SetThis(this);  // 设置当前协程为主协程
        m_state = RUNNING;  // 主协程初始状态为RUNNING

        // 获取当前上下文，保存到m_ctx，失败则断言
        if(getcontext(&m_ctx))
        {
            SYLAR_ASSERT2(false, "getcontext");
        }
        ++s_fiber_count;  // 当前协程数量增加
        m_id = s_fiber_id++;  // 生成新的协程id

        // 记录日志，输出协程id
        SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber() main id = " << m_id;
    }

    /**
     * @brief 设置当前线程正在执行的协程
     * @param f 指向当前协程的指针
     */
    void Fiber::SetThis(Fiber *f)
    {
        t_fiber = f;
    }

    /**
     * @brief 获取当前协程，如果当前线程没有协程，则初始化主协程
     * @return 当前协程的智能指针
     */
    Fiber::ptr Fiber::GetThis()
    {
        if(t_fiber)
        {
            // 返回当前协程的智能指针
            return t_fiber->shared_from_this();
        }

        // 当前线程没有主协程，创建一个
        Fiber::ptr main_fiber(new Fiber);
        SYLAR_ASSERT(t_fiber == main_fiber.get());  // 断言当前协程就是刚创建的主协程
        t_thread_fiber = main_fiber;  // 保存线程的主协程
        return t_fiber->shared_from_this();
    }

    /**
     * @brief 带回调函数的构造函数，用于创建普通协程
     * @param cb 协程的回调函数
     * @param stacksize 协程栈大小
     * @param run_in_scheduler 协程是否参与调度器调度
     */
    Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool run_in_scheduler)
        : m_id(s_fiber_id++)  // 初始化协程id
        , m_cb(cb)  // 保存回调函数
        , m_runInScheduler(run_in_scheduler)  // 是否在调度器中调度
        {
            ++s_fiber_count;  // 增加协程计数
            m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();  // 设置栈大小
            m_stack = StackAllocator::Alloc(m_stacksize);  // 分配协程栈内存

            if(getcontext(&m_ctx))  // 获取当前上下文，保存到m_ctx
            {
                SYLAR_ASSERT2(false, "getcontext");
            }

            m_ctx.uc_link = nullptr;  // 协程结束时不自动切换到其他上下文
            m_ctx.uc_stack.ss_sp = m_stack;  // 设置协程栈
            m_ctx.uc_stack.ss_size = m_stacksize;  // 设置栈大小

            // 设置协程入口函数为Fiber::MainFunc
            makecontext(&m_ctx, &Fiber::MainFunc, 0);
            
            SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber() id = " << m_id;  // 记录协程创建日志
        }

    /**
     * @brief 析构函数，释放资源
     * 普通协程会释放栈内存，主协程只释放cb
     */
    Fiber::~Fiber()
    {
        SYLAR_LOG_DEBUG(g_logger) << "Fiber::~Fiber() id = " <<m_id;
        --s_fiber_count;  // 协程计数减1

        if(m_stack)
        {
            // 普通协程，释放栈内存
            SYLAR_ASSERT(m_state == TERM);  // 断言协程已终止
            StackAllocator::Dealloc(m_stack, m_stacksize);  // 释放栈
            SYLAR_LOG_DEBUG(g_logger) << "dealloc stack, id = " << m_id;
        }
        else
        {
            // 主协程不需要释放栈，检查cb和状态
            SYLAR_ASSERT(!m_cb);  // 主协程没有回调函数
            SYLAR_ASSERT(m_state == RUNNING);  // 主协程应该仍在运行

            Fiber* cur = t_fiber;  // 当前协程就是主协程
            if(cur == this)
            {
                SetThis(nullptr);  // 将当前协程置空
            }
        }
    }

    /**
     * @brief 重置协程的回调函数，并重新初始化上下文
     * @param cb 新的回调函数
     */
    void Fiber::reset(std::function<void()> cb)
    {
        SYLAR_ASSERT(m_stack);  // 必须有栈
        SYLAR_ASSERT(m_state == TERM);  // 协程必须是终止状态才能重置
        m_cb = cb;  // 设置新的回调函数
        if(getcontext(&m_ctx))
        {
            SYLAR_ASSERT2(false, "getcontext");
        }

        m_ctx.uc_link = nullptr;  // 协程结束时不自动切换到其他上下文
        m_ctx.uc_stack.ss_sp = m_stack;  // 设置协程栈
        m_ctx.uc_stack.ss_size = m_stacksize;  // 设置栈大小

        // 设置协程入口函数为Fiber::MainFunc
        makecontext(&m_ctx, &Fiber::MainFunc, 0);
        m_state = READY;  // 重置后的协程状态为READY
    }

    /**
     * @brief 恢复协程的执行
     */
    void Fiber::resume()
    {
        SYLAR_ASSERT(m_state != TERM && m_state != RUNNING);  // 协程不能是终止或正在运行状态
        SetThis(this);  // 将当前协程设置为正在运行的协程
        m_state = RUNNING;  // 设置状态为RUNNING

        // 如果协程参与调度器调度，与调度器的主协程交换上下文
        if(m_runInScheduler)
        {
            if(swapcontext(&(Scheduler::GetMainFiber()->m_ctx), &m_ctx))
            {
                SYLAR_ASSERT2(false, "swapcontext");
            }
        }
        else
        {
            // 如果不参与调度器调度，与线程的主协程交换上下文
            if(swapcontext(&(t_thread_fiber->m_ctx), &m_ctx))
            {
                SYLAR_ASSERT(false, "swapcontext");
            }
        }
    }

    /**
     * @brief 暂停当前协程的执行，切换回主协程
     */
    void Fiber::yield()
    {
        SYLAR_ASSERT(m_state == RUNNING || m_state == TERM);  // 协程必须在运行或终止状态时才能yield
        SetThis(t_thread_fiber.get());  // 切换回主协程
        if(m_state != TERM)
        {
            m_state = READY;  // 如果协程未终止，将其状态置为READY
        }

        // 如果协程参与调度器调度，与调度器的主协程交换上下文
        if(m_runInScheduler)
        {
            if(swapcontext(&m_ctx, &(Scheduler::GetMainFiber()->m_ctx)))
            {
                SYLAR_ASSERT(false, "swapcontext");
            }
        }
        else
        {
            // 如果不参与调度器调度，与线程的主协程交换上下文
            if(swapcontext(&m_ctx, &t_thread_fiber->m_ctx))
            {
                SYLAR_ASSERT(false, "swapcontext");
            }
        }
    }
}
```

