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
    initPool();
}

void ThreadPool::initPool()
{
    for(int i = 0; i < m_vec_thread.size(); ++i) {
        m_vec_thread.at(i) = std::thread(std::bind(&ThreadPool::executeTask, this));
    }
}

ThreadPool::~ThreadPool()
{
    if(m_bShutdown == false)
    {
        shutdown();
    }
}

void ThreadPool::addTask(Task task)
{
    std::unique_lock<std::mutex> lock_task_queue(m_task_mutex);
    m_task_queue.push(task);
    m_condition_variable_task.notify_one();
}

void ThreadPool::executeTask()
{
    while(m_bShutdown != true)
    {
        std::unique_lock<std::mutex> lock_thread(m_thread_mutex);
        if(m_task_queue.empty()){
            m_condition_variable_task.wait(lock_thread);
        }

        std::unique_lock<std::mutex> lock_task_queue(m_task_mutex);
        if(!m_task_queue.empty()){
            std::function<void()> func = m_task_queue.front();
            m_task_queue.pop();

            func();
        }

    }    
}

void ThreadPool::shutdown()
{
    std::cout << __func__ << __LINE__ << std::endl;

    m_bShutdown = true;  
    m_condition_variable_task.notify_all();
    for(int i = 0; i < m_vec_thread.size(); ++i) {
        if(m_vec_thread.at(i).joinable()){
            m_vec_thread.at(i).join();
        }
    }
}