#ifndef WORKDER_THREAD_HPP__
#define WORKDER_THREAD_HPP__

#include <chrono>
#include <queue>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

class WorkerThread
{
private:
    struct Task {
        std::chrono::time_point<std::chrono::steady_clock> when;
        std::function<void()> what;
    };

    friend bool operator<(const Task& lhs, const Task& rhs);

    std::atomic<bool> m_isTerminating;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::thread m_Thread;
    std::priority_queue<Task> m_Tasks;

    void threadLoop();

public:
    WorkerThread(/* args */);
    ~WorkerThread();

    void schedule(std::function<void()> task, std::chrono::microseconds delay);
    void cancelAll();
};
#endif