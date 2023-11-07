#include "threadpool.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define false 0
#define true 1

Thread_Pool_T* init_thread_pool(int min_thread_num, int max_thread_num, int queue_max_num)
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
        memset(queue_task, 0 , sizeof(sizeof(Queue_Task_T)));

        queue_task->queue_max_size = queue_max_num;
        queue_task->queue_size = 0;
        queue_task->queue_front = 0;
        queue_task->queue_rear = 0;
        queue_task->tasks = (Thread_Task_T*)malloc(queue_max_num*sizeof(Thread_Task_T));

        if( 0 != pthread_mutex_init(&queue_task->mutex_queue, NULL)
            || 0 != pthread_cond_init(&queue_task->cond_queue_not_empty,NULL)
            || 0 != pthread_cond_init(&queue_task->cond_queue_not_full,NULL)){
                printf("Init task queue mutex or cond error\n");
                break;
        }

        pool->queue_task = queue_task;

        // init work thread
        pool->work_pthread = (pthread_t*)malloc(min_thread_num*sizeof(pthread_t));
        if(pool->work_pthread == NULL) {
            printf("Malloc work pthread array error\n");
            break; 
        }
        
        memset(pool->work_pthread, 0 , sizeof(min_thread_num*sizeof(pthread_t)));

        for(i = 0; i < min_thread_num; ++i) {
            pthread_create(&(pool->work_pthread[i]), NULL, work_func, (void*)pool);
            printf("Start a thread : 0x%x\n", (unsigned int)(pool->work_pthread[i]));
        }

        pthread_create(&pool->manage_pthread, NULL, manage_func, (void*)pool);

        pool->shutdown = false;
        return pool;

    } while (0);
    

}

void destroy_thread_pool(Thread_Pool_T* pool)
{
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
            free(pool->queue_task->tasks);
            pool->queue_task->tasks = NULL;
        }
        pthread_mutex_destroy(&pool->queue_task->mutex_queue);
        pthread_cond_destroy(&pool->queue_task->cond_queue_not_empty);
        pthread_cond_destroy(&pool->queue_task->cond_queue_not_full);


    }

    free(pool->queue_task);
    pool->queue_task = NULL;

    free(pool);
    pool = NULL;

    return;
}

void* work_func(void* threadpool)
{

}

void* manage_func(void* threadpool)
{

}