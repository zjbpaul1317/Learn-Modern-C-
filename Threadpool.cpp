#include <pthread.h>
#include <queue>
#include <iostream>
#include <string.h>
#include <exception>
#include <unistd.h>

const int NUMBER = 2;

using callback = void (*)(void *arg);

class Task
{
public:
    callback fun;
    void *arg;

public:
    Task()
    {
        fun = nullptr;
        arg = nullptr;
    }

    Task(callback f, void *arg)
    {
        fun = f;
        this->arg = arg;
    }
};

class Task_queue
{
private:
    pthread_mutex_t m_mutex;
    std::queue<Task> m_mutex;

public:
    Task_queue();
    ~Task_queue();

    // 添加任务
    void add_task(Task &task);
    void add_task(callback f, void *arg);

    // 取出任务
    Task &get_task();

    int task_size()
    {
        return m_queue.size();
    }
};

class Thread_pool
{
private:
    pthread_mutex_t m_mutex_pool;
    pthread_cond_t m_not_empty; // 条件变量，用来唤醒等待的线程，假设队列是无限的
    pthread_t *thread_compose;
    pthread_t m_manager_thread;
    Task_queue *m_task_queue;
    int m_min_num;         // 最小线程数
    int m_max_num;         // 最大线程数
    int m_busy_num;        // 工作的线程数
    int m_live_num;        // 存活的线程数一
    int m_destory_num;     // 待销毁的线程数
    bool shutdown = false; // 是否销毁

    // 在使用pthread_create时候，第三个参数的函数必须是静态的
    // 要在静态函数中使用类的成员(成员函数和变量)只能通过两种方式
    // 1.通过类的静态对象，单例模式中使用类的全局唯一实例来访问类成员
    // 2.类对象作为参数传递给该静态函数

    // 管理线程的任务函数，形参是线程池类型的参数
    static void *manager(void *arg);

    // 工作线程的任务函数
    static void *worker(void *arg);

    // 销毁线程池
    void destory_thread();

public:
    Thread_pool(int min, int max);
    ~Thread_pool();

    void add_task(Task task);
    int get_busy_num();
    int get_live_num();
};

Task_queue::Task_queue()
{
    pthread_mutex_init(&m_mutex, nullptr);
}

Task_queue::~Task_queue()
{
    pthread_mutex_destroy(&m_mutex);
}

void Task_queue::add_task(Task &task)
{
    pthread_mutex_lock(&m_mutex);
    m_queue.push(task);
    pthread_mutex_unlock(&m_mutex);
}

void Task_queue::add_task(callback f, void *arg)
{
}

Task &Task_queue::get_task()
{
    Task t;
    pthread_mutex_lock(&m_mutex);
    if (m_queue.size() > 0)
    {
        t = m_queue.front();
        m_queue.pop();
    }
    pthread_mutex_unlock(&m_mutex);
    return t;
}