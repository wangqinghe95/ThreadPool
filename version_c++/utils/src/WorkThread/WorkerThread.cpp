#include "WorkerThread.hpp"

using std::lock_guard;
using std::unique_lock;
using std::mutex;
using std::chrono::steady_clock;
using std::condition_variable;
using std::priority_queue;

bool operator<(const WorkerThread::Task& lhs, const WorkerThread::Task& rhs)
{
    return lhs.when > rhs.when;
}

WorkerThread::WorkerThread() : m_isTerminating(false), m_Thread(&WorkerThread::threadLoop, this) {}

WorkerThread::~WorkerThread()
{
    {
        lock_guard<mutex> lk(m_mutex);
        m_isTerminating = true;
        m_condition.notify_one();
    }

    m_Thread.join();
}

void WorkerThread::schedule(std::function<void()> task, std::chrono::microseconds delay)
{
    auto when = steady_clock::now() + delay;
    lock_guard<mutex> lk(m_mutex);
    m_Tasks.push(Task({when,task}));
    m_condition.notify_one();
}
void WorkerThread::cancelAll()
{
    lock_guard<mutex> lk(m_mutex);
    priority_queue<Task>().swap(m_Tasks);
}

void WorkerThread::threadLoop()
{
    while (m_isTerminating)
    {
        unique_lock<mutex> lk(m_mutex);
        if(m_Tasks.empty()) {
            m_condition.wait(lk);
            continue;
        }

        auto task = m_Tasks.top();
        if(task.when > steady_clock::now()) {
            m_condition.wait_until(lk, task.when);
            continue;
        }

        m_Tasks.pop();
        lk.unlock();
        task.what();
    }
    
}