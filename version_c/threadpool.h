#ifndef THREAD_POOL_H__
#define THREAD_POOL_H__

#include <pthread.h>

typedef void* (*CallbackFunction)(void* args);


// a struct for describing Task
typedef struct Thread_Task_Tag
{
    CallbackFunction func_cb;
    void* args; 
}Thread_Task_T;

typedef struct Queue_Task_Tag
{
    Thread_Task_T* tasks;
    int queue_max_size;
    int queue_size;
    int queue_front;
    int queue_rear;

    pthread_cond_t cond_queue_not_full;  // indicate the queue_task is not full, can add task
    pthread_cond_t cond_queue_not_empty; // indicate the queue_task is not empty, can execuate work
}Queue_Task_T;

typedef struct Thread_Pool_Tag
{
    int pool_max_size;
    int pool_min_size;

    int busy_thread_num;    // thread number for working
    int live_thread_num;    // total thread number for live
    int wait_exit_num;      // the number of thread waitting to exit

    pthread_mutex_t lock_work_thread;   // a lock for to be working thread
    pthread_mutex_t lock_busy_thread;   // a lock for working

    pthread_t *work_pthread;        // a array of thread for thread pool
    pthread_t manage_pthread;       // manage thread pool thread, add or delete
    Queue_Task_T* queue_task;       // a array of task need to be done

    int shutdown;
}Thread_Pool_T;


// init thread pool
Thread_Pool_T* initThreadPool(int min_thread_num, int max_thread_num, int queue_max_num);
void destroyThreadPool(Thread_Pool_T* pool);

// add a task into task queue
void addTask2Queue(Thread_Pool_T* pool, CallbackFunction Task, void* args);

void* workFunc(void* threadpool);

void* manageThreadPool(void* threadpool);

int getTaskQueueSize(Queue_Task_T* task_queue);

int getTaskFromQueue(Queue_Task_T* task_queue, Thread_Task_T* task);


#endif