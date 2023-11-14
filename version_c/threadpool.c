#include "threadpool.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

#define false 0
#define true 1

#define DEFAULT_TIME 5
#define MIN_WAIT_TASKNUM 6
#define DEFAULT_THREAD_VARY 6

int getTaskQueueSize(Queue_Task_T* task_queue)
{
    int size = 0;

    size = task_queue->queue_size;
    return size;
}

int getTaskFromQueue(Queue_Task_T* task_queue, Thread_Task_T* task)
{
    task->func_cb = task_queue->tasks[task_queue->queue_front].func_cb;
    task->args = task_queue->tasks[task_queue->queue_front].args;
    task_queue->queue_front = (task_queue->queue_front+1)%task_queue->queue_max_size;
    task_queue->queue_size--;

    pthread_cond_broadcast(&task_queue->cond_queue_not_full);

    return 1;
}

static int addTaskIntoQueue(Queue_Task_T* task_queue, Thread_Task_T* task)
{
    task_queue->tasks[task_queue->queue_rear].func_cb = task->func_cb;
    task_queue->tasks[task_queue->queue_rear].args = task->args;
    task_queue->queue_rear = (task_queue->queue_rear+1)%task_queue->queue_max_size;
    task_queue->queue_size++;

    pthread_cond_signal(&task_queue->cond_queue_not_empty);
    return 1;
}

static int bTaskQueueFull(Queue_Task_T* task_queue)
{
    if(task_queue->queue_size == task_queue->queue_max_size) {
        return true;
    }
    else {
        return false;
    }
}

Thread_Pool_T* initThreadPool(int min_thread_num, int max_thread_num, int queue_max_num)
{
    int i = 0;
    Thread_Pool_T* pool = NULL;
    do
    {
        // init pool
        pool = (Thread_Pool_T*)malloc(sizeof(Thread_Pool_T));
        if(pool == NULL) {
            printf("Malloc for Thread_Pool error");
            break;
        }

        pool->pool_max_size = max_thread_num;
        pool->pool_min_size = min_thread_num;
        pool->busy_thread_num = 0;
        pool->live_thread_num = min_thread_num;

        if(0 != pthread_mutex_init(&pool->lock_work_thread, NULL) || 
            0 != pthread_mutex_init(&pool->lock_busy_thread, NULL)) {
                printf("Init thread pool mutex error\n");
                break;
        }

        // init queue task
        Queue_Task_T* queue_task = (Queue_Task_T*)malloc(sizeof(Queue_Task_T));
        if(NULL == queue_task) {
            printf("Malloc queue task error\n");
            break;
        }
        memset(queue_task, 0 , sizeof(Queue_Task_T));

        queue_task->queue_max_size = queue_max_num;
        queue_task->queue_size = 0;
        queue_task->queue_front = 0;
        queue_task->queue_rear = 0;
        queue_task->tasks = (Thread_Task_T*)malloc(queue_max_num*sizeof(Thread_Task_T));

        memset(queue_task->tasks, 0 , queue_max_num*sizeof(Thread_Task_T));

        if( 0 != pthread_cond_init(&queue_task->cond_queue_not_empty,NULL)
            || 0 != pthread_cond_init(&queue_task->cond_queue_not_full,NULL)){
                printf("Init task queue mutex or cond error\n");
                break;
        }

        pool->queue_task = queue_task;

        // init work thread
        pool->work_pthread = (pthread_t*)malloc(max_thread_num*sizeof(pthread_t));
        if(pool->work_pthread == NULL) {
            printf("Malloc work pthread array error\n");
            break; 
        }
        
        memset(pool->work_pthread, 0 , max_thread_num*sizeof(pthread_t));

        for(i = 0; i < min_thread_num; ++i) {
            pthread_create(&(pool->work_pthread[i]), NULL, workFunc, (void*)pool);
            printf("Start a thread : 0x%x\n", (unsigned int)(pool->work_pthread[i]));
        }

        pthread_create(&pool->manage_pthread, NULL, manageThreadPool, (void*)pool);

        pool->shutdown = false;
        return pool;

    } while (0);
    
    destroyThreadPool(pool);

    return NULL;
}

void destroyThreadPool(Thread_Pool_T* pool)
{
    printf("%s\n", __func__);
    int i = 0;
    if( pool == NULL) {
        return;
    }

    pool->shutdown = true;
    pthread_join(pool->manage_pthread, NULL);

    for(i = 0; i < pool->live_thread_num; ++i) {
        pthread_cond_broadcast(&(pool->queue_task->cond_queue_not_empty));
    }

    for(i = 0 ; i < pool->live_thread_num; ++i) {
        pthread_join((pool->work_pthread[i]), NULL);
    }

    pthread_mutex_destroy(&(pool->lock_busy_thread));
    pthread_mutex_destroy(&(pool->lock_work_thread));

    if(pool->queue_task != NULL) {
        if(pool->queue_task->tasks != NULL) {
            printf("free %p\n",pool->queue_task->tasks);
            free(pool->queue_task->tasks);
            pool->queue_task->tasks = NULL;
        }
        pthread_cond_destroy(&pool->queue_task->cond_queue_not_empty);
        pthread_cond_destroy(&pool->queue_task->cond_queue_not_full);
    }

    free(pool->queue_task);
    pool->queue_task = NULL;

    free(pool->work_pthread);
    pool->work_pthread = NULL;

    free(pool);
    pool = NULL;

    return;
}

void* workFunc(void* threadpool)
{
    int i = 0;
    Thread_Pool_T* pool = (Thread_Pool_T*)threadpool;

    while (1)
    {
        printf("thread 0x%lu wait working\n", pthread_self());
        pthread_mutex_lock(&pool->lock_work_thread);
        while (0 == getTaskQueueSize(pool->queue_task) && pool->shutdown == false)
        {
            pthread_cond_wait(&(pool->queue_task->cond_queue_not_empty), &pool->lock_work_thread);
            printf("thread 0x%lu get lock\n", pthread_self());

            if(pool->wait_exit_num > 0) {
                pool->wait_exit_num--;
                if(pool->live_thread_num > pool->pool_min_size) {
                    printf("Thread 0x%x exited\n",(unsigned int)pthread_self());
                    pool->live_thread_num--;
                    pthread_mutex_unlock(&(pool->lock_work_thread));
                    pthread_exit(NULL);
                }
            }
        }

        if(pool->shutdown) {
            pthread_mutex_unlock(&pool->lock_work_thread);
            printf("thread 0x%lu exit\n", pthread_self());
            pthread_exit(NULL);
        }

        Thread_Task_T task;
        getTaskFromQueue(pool->queue_task,&task);

        pthread_mutex_unlock(&pool->lock_work_thread);

        printf("Thread 0x%lu start working\n",pthread_self());
        (*(task.func_cb))(task.args);

        printf("Thread 0x%lu end working\n",pthread_self());
        
    }
    
    pthread_exit(NULL);
}

static int isThreadAlive(pthread_t tid) 
{
    int kill_res = pthread_kill(tid, 0);
    if( ESRCH == kill_res ) {
        return false;
    }

    return true;
}

void* manageThreadPool(void* threadpool)
{
    Thread_Pool_T *pool = (Thread_Pool_T*)threadpool;
    while(!pool->shutdown) {

        sleep(DEFAULT_TIME);
        printf("Thread manage check 0x%lu\n", pthread_self());

        pthread_mutex_lock(&pool->lock_work_thread);
        int queue_size = pool->queue_task->queue_size;
        int live_thread_num = pool->live_thread_num;
        pthread_mutex_unlock(&pool->lock_work_thread);

        pthread_mutex_lock(&(pool->lock_busy_thread));
        int busy_thread_num = pool->busy_thread_num;
        pthread_mutex_unlock(&(pool->lock_busy_thread));

        // add thread num
        if(queue_size > MIN_WAIT_TASKNUM && live_thread_num < pool->pool_max_size) {
            pthread_mutex_lock(&(pool->lock_work_thread));
            int add = 0;
            for(int i = 0; i < pool->pool_max_size ; ++i) {
                if( add >= DEFAULT_THREAD_VARY) {
                    break;
                }

                if(pool->live_thread_num >= pool->pool_max_size) {
                    break;
                }

                if(pool->work_pthread[i] == 0 || !isThreadAlive(pool->work_pthread[i])) {
                    pthread_create(&(pool->work_pthread[i]), NULL, workFunc, (void*)pool);
                    printf("%s add thread:0x%x\n", __func__,  (unsigned int)pool->work_pthread[i]);
                    add++;
                    pool->live_thread_num++;
                }
            }
            pthread_mutex_unlock(&(pool->lock_work_thread));
        }

        // remove thread
        if(busy_thread_num * 2 < live_thread_num && live_thread_num > pool->pool_min_size) {
            printf("%s remove threads\n",__func__);
            pthread_mutex_unlock(&(pool->lock_work_thread));
            pool->wait_exit_num = DEFAULT_THREAD_VARY;
            pthread_mutex_unlock(&(pool->lock_work_thread));
            
            for(int i = 0; i < DEFAULT_THREAD_VARY; i++) {
                pthread_cond_signal(&(pool->queue_task->cond_queue_not_empty));
            }
        }
    }
    return NULL;
}

void addTask2Queue(Thread_Pool_T* pool, CallbackFunction func, void* args)
{
    printf("Start add task into task_queue\n");

    pthread_mutex_lock(&(pool->lock_work_thread));

    while(bTaskQueueFull(pool->queue_task) && !pool->shutdown)
    {
        printf("TaskQueue full, please wait....\n");
        pthread_cond_wait(&(pool->queue_task->cond_queue_not_full), &pool->lock_work_thread);
    }

    printf("TaskQueue not full, queue size %d, add task\n", pool->queue_task->queue_size);
    Thread_Task_T task;
    task.func_cb = func;
    task.args = args;

    addTaskIntoQueue(pool->queue_task, &task);

    pthread_mutex_unlock(&(pool->lock_work_thread));

    printf("add task end\n");
}