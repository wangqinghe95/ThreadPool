#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include<queue>
#include<thread>
#include<mutex>
#include<condition_variable>

class ThreadPool
{
public:
    using Task = std::function<void()>;


public:
    ThreadPool(int min_num = 2, int max_num = 20, int queue_size_max = 30);
    ~ThreadPool();

    void initPool();

    void addTask(Task task);
    void executeTask();

    void shutdown();

private:
    int m_min_thr_num;
    int m_max_thr_num;
    int m_live_thr_num;
    int m_busy_thr_num;
    int m_wait_exit_thr_num;

    int m_queue_max_size;

    std::mutex m_thread_mutex;  // One thread can work if there is a task in the multiply threads conditions
    std::condition_variable m_condition_variable_task;  // Nofity threads if there is a task, if no, please sleep for all threads

    std::mutex m_task_mutex;

    std::vector<std::thread> m_vec_thread;
    std::queue<Task> m_task_queue;

    bool m_bShutdown;
};

#endif