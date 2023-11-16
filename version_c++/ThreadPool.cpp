#include "ThreadPool.hpp"
#include <iostream>


ThreadPool::ThreadPool(int min_num, int max_num, int queue_size_max)
                    : m_min_thr_num(min_num)
                    , m_max_thr_num(max_num)
                    , m_queue_max_size(queue_size_max)
                    , m_wait_exit_thr_num(0)
                    , m_vec_thread(std::vector<std::thread>(min_num))
                    , m_bShutdown(false)
{
    for(int i = 0; i < m_vec_thread.size(); ++i) {
        m_vec_thread.at(i) = std::thread(std::bind(&ThreadPool::run, this));
    }
}

ThreadPool::~ThreadPool()
{
    if(m_bShutdown == false)
    {
        shutdown();
    }
}

void ThreadPool::run()
{
    while (m_bShutdown != true)
    {
        std::unique_lock<std::mutex> lk(m_thread_mutex);
        while(m_task_queue.empty()){
            m_condition_variable_task.wait(lk);
        }

        Task task;
        if(true == m_task_queue.dequeue(task)) {
            task();
        }   
    }
    
}

void ThreadPool::shutdown()
{
    m_bShutdown = true;  
    m_condition_variable_task.notify_all();
    for(int i = 0; i < m_vec_thread.size(); ++i) {
        if(m_vec_thread.at(i).joinable()){
            m_vec_thread.at(i).join();
        }
    }

    std::cout << "threadpool destory"<< std::endl;
}