#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <utility>

template <typename T>
class TaskQueue
{
private:
    std::queue<T> m_Tasks;
    std::mutex m_lock_queue;
public:
    TaskQueue(){}
    TaskQueue(const TaskQueue&& other){}
    ~TaskQueue(){
        std::queue<T>().swap(m_Tasks);
    }
    bool empty(){
        std::unique_lock<std::mutex> lk(m_lock_queue);
        return m_Tasks.empty();
    }

    int size() {
        std::unique_lock<std::mutex> lk(m_lock_queue);
        return m_Tasks.size();
    }

    void enqueue(T& t) {
        std::unique_lock<std::mutex> lk(m_lock_queue);
        m_Tasks.emplace(t);
    }

    bool dequeue(T &t) {
        std::unique_lock<std::mutex> lk(m_lock_queue);
        if(m_Tasks.empty()) {
            return false;
        }

        t = std::move(m_Tasks.front());
        m_Tasks.pop();

        return true;
    }
};

class ThreadPool
{
public:
    using Task = std::function<void()>;

public:
    ThreadPool(int min_num = 2, int max_num = 20, int queue_size_max = 30);
    ~ThreadPool();
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool &&) = delete;

    void shutdown();
    void run();

    template <typename F, typename... Args>
    auto submit(F&& f, Args &&...args) -> std::future<decltype(f(args...))>
    {
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

        std::function<void()> warpper_func = [task_ptr](){
            (*task_ptr)();
        };

        m_task_queue.enqueue(warpper_func);
        m_condition_variable_task.notify_one();

        return task_ptr->get_future();
    }

private:
    int m_min_thr_num;
    int m_max_thr_num;
    // int m_live_thr_num;
    // int m_busy_thr_num;
    int m_wait_exit_thr_num;

    int m_queue_max_size;

    std::mutex m_thread_mutex;  // One thread can work if there is a task in the multiply threads conditions
    std::condition_variable m_condition_variable_task;  // Nofity threads if there is a task, if no, please sleep for all threads

    std::vector<std::thread> m_vec_thread;
    TaskQueue<Task> m_task_queue;

    bool m_bShutdown;
};

#endif