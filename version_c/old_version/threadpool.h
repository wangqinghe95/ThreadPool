#ifndef THREADPOOL_H__
#define THREADPOOL_H__

#include <pthread.h>
#include <stdio.h>

typedef struct threadpool_t threadpool_t;
typedef void* (*Task)(void* arg);

typedef struct 
{
    Task function;
    void* args;
} threadpool_task_t;

struct threadpool_t
{
    pthread_mutex_t lock;   // make sure that only one thread can execuate job in multuply threads
    pthread_mutex_t busy_thr_lock;
    pthread_cond_t queue_not_full;  // block adding task thread if task queue is full
    pthread_cond_t queue_not_empty; // notify working thread if task queue is not empty

    pthread_t *threads;     // a array with working thread
    pthread_t manage_thread;  // a thread to manage threadpool, for example adjusting number of thread for pool

    threadpool_task_t* task_queue;

    int min_thr_num;        // the min num of the array with working thread
    int max_thr_num;        // the max num of the array with working thread
    int live_thr_num;
    int busy_thr_num;
    int wait_exit_thr_num;

    int queue_front;
    int queue_rear;
    int queue_size;
    int queue_max_size;

    int shutdown;      // a sign to indicator whether the pool is using

};

threadpool_t* threadpool_create(int min_thr_num, int max_thr_num, int queue_max_size);

int threadpool_add(threadpool_t* pool, Task task, void* arg);

int threadpool_destory(threadpool_t *pool);

int threadpool_all_threadnum(threadpool_t *pool);

int threadpool_busy_threadnum(threadpool_t *pool);

#endif