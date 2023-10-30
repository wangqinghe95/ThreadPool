#include "threadpool.h"

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#define true 1
#define false 0

#define DEFAULT_CHECK_TIME 10
#define MIN_WAIT_TASK_NUM 10
#define DEFAULT_THREAD_VARY 10

void* thread_work(void* threadpool);
void* manage_thread(void* threadpool);
int threadpool_free(threadpool_t *pool);
int is_live_thread(pthread_t tid);

threadpool_t* threadpool_create(int min_thr_num, int max_thr_num, int queue_max_size)
{
    int i = 0;
    threadpool_t *pool = NULL;
    do {
        pool = (threadpool_t*)malloc(sizeof(threadpool_t));
        if(NULL == pool) {
            printf("Malloc threadpool failed\n");
            break;
        }

        pool->min_thr_num = min_thr_num;
        pool->max_thr_num = max_thr_num;
        pool->live_thr_num = min_thr_num;
        pool->busy_thr_num = 0;

        pool->queue_max_size = queue_max_size;
        pool->queue_size = 0;
        pool->queue_front = 0;
        pool->queue_rear = 0;
        pool->shutdown = false;

        // create working thread array and set zero
        pool->threads = (pthread_t *)malloc(sizeof(pthread_t)*max_thr_num);
        if(NULL == pool->threads) {
            printf("Malloc threads failed\n");
            break;
        }
        memset(pool->threads, 0, sizeof(pthread_t)*max_thr_num);

        // malloc free for task queue
        pool->task_queue = (threadpool_task_t*)malloc(sizeof(threadpool_task_t)*queue_max_size);
        if(NULL == pool->task_queue) {
            printf("Malloc task queue failed\n");
            break;
        }

        memset(pool->task_queue, 0, sizeof(pthread_t)*max_thr_num);


        if( 0 != pthread_mutex_init(&(pool->lock), NULL) 
             || 0 != pthread_mutex_init(&(pool->busy_thr_lock), NULL)
             || 0 != pthread_cond_init(&(pool->queue_not_empty), NULL)
             || 0 != pthread_cond_init(&(pool->queue_not_full), NULL))
        {
            printf("init lock or cond failed\n");
            break;
        }

        // create threads
        for(i = 0; i < min_thr_num; ++i) {
            pthread_create(&(pool->threads[i]), NULL, thread_work, (void*)pool);
            printf("start thread 0x%x...\n", (unsigned int)pool->threads[i]);
        }

        pthread_create(&(pool->manage_thread), NULL, manage_thread, (void*)pool);

        return pool;

    }while(0);

    threadpool_destory(pool);

    return NULL;
}
int threadpool_destory(threadpool_t *pool)
{
    int i;
    if (pool == NULL) {
        return -1;
    }
    pool->shutdown = true;

    /*先销毁管理线程*/
    pthread_join(pool->manage_thread, NULL);

    for (i = 0; i < pool->live_thr_num; i++) {
        /*通知所有的空闲线程*/
        pthread_cond_broadcast(&(pool->queue_not_empty));
    }
    for (i = 0; i < pool->live_thr_num; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    threadpool_free(pool);

    return 0;
}

int threadpool_free(threadpool_t *pool)
{
    if(NULL == pool) {
        return -1;
    }

    if( pool->task_queue) {
        free(pool->task_queue);
    }

    if(pool->threads){
        free(pool->threads);
        pthread_mutex_lock(&(pool->lock));
        pthread_mutex_unlock(&(pool->lock));
        pthread_mutex_destroy(&(pool->lock));
        
        pthread_mutex_lock(&(pool->busy_thr_lock));
        pthread_mutex_unlock(&(pool->busy_thr_lock));
        pthread_mutex_destroy(&(pool->busy_thr_lock));

        pthread_cond_destroy(&(pool->queue_not_empty));
        pthread_cond_destroy(&(pool->queue_not_full));
    }

    free(pool);
    pool = NULL;

    return 0;
}

int threadpool_add(threadpool_t* pool, Task task, void* arg)
{
    printf("thread 0x%x threadpool_add\n", (unsigned int)pthread_self());
    pthread_mutex_lock(&(pool->lock));

    while(pool->queue_size == pool->queue_max_size && (!pool->shutdown)) {
        pthread_cond_wait(&(pool->queue_not_full), &(pool->lock));  // wait signal of the queue_not_full if the task queue is full
    }

    if(pool->shutdown) {
        pthread_mutex_unlock(&(pool->lock));
    }

    // first, clear args of queue rear
    if(pool->task_queue[pool->queue_rear].args != NULL) {
        free(pool->task_queue[pool->queue_rear].args);
        pool->task_queue[pool->queue_rear].args = NULL;
    }

    // second, add a task into task_queue in queue rear

    pool->task_queue[pool->queue_rear].function = task;
    pool->task_queue[pool->queue_rear].args = arg;
    pool->queue_rear = (pool->queue_rear + 1) % pool->queue_max_size;   // simulate circle queue
    pool->queue_size++;

    // signal a waiting thread to execuate task
    pthread_cond_signal(&(pool->queue_not_empty));
    pthread_mutex_unlock(&(pool->lock));
    
    printf("thread 0x%x threadpool_add finish\n", (unsigned int)pthread_self());

    return 0;
}

int threadpool_all_threadnum(threadpool_t *pool)
{
    int all_threadnum = -1;
    pthread_mutex_lock(&(pool->lock));
    all_threadnum = pool->live_thr_num;
    pthread_mutex_unlock(&(pool->lock));
    return all_threadnum;
}

int threadpool_busy_threadnum(threadpool_t *pool)
{
    int busy_threadnum = -1;
    pthread_mutex_lock(&(pool->busy_thr_lock));
    busy_threadnum = pool->busy_thr_num;
    pthread_mutex_unlock(&(pool->busy_thr_lock));
    return busy_threadnum;    
}

int is_live_thread(pthread_t tid)
{
    int kill_res = pthread_kill(tid, 0);
    if( ESRCH == kill_res ) {
        return false;
    }

    return true;
}

void* thread_work(void* threadpool)
{
    threadpool_t* pool = (threadpool_t *)threadpool;
    threadpool_task_t task;

    while (1)
    {
        pthread_mutex_lock(&(pool->lock));

        while ((pool->queue_size == 0) && (!pool->shutdown))
        {
            printf("thread 0x%x is waiting\n", (unsigned int)pthread_self());

            pthread_cond_wait(&(pool->queue_not_empty), &(pool->lock)); // thread will be waiting until receive a signal of queue_not_empty if there are no tasks in task_queue, 

            printf("thread 0x%x get lock\n", (unsigned int)pthread_self());

            // exit thread self
            if(pool->wait_exit_thr_num > 0) {
                pool->wait_exit_thr_num--;

                if(pool->live_thr_num > pool->min_thr_num) {
                    printf("thread 0x%x is exiting\n", (unsigned int)pthread_self());
                    pool->live_thr_num--;
                    pthread_mutex_unlock(&(pool->lock));
                    pthread_exit(NULL);
                }

            }
        }

        printf("thread 0x%x execuate work\n", (unsigned int)pthread_self());

        if(pool->shutdown) {
            pthread_mutex_unlock(&(pool->lock));
            printf("thead 0x%x is existing\n", (unsigned int)pthread_self());
            pthread_exit(NULL);
        }
        
        task.function = pool->task_queue[pool->queue_front].function;
        task.args = pool->task_queue[pool->queue_front].args;

        pool->queue_front = (pool->queue_front + 1) % pool->queue_max_size;
        pool->queue_size--;

        pthread_cond_broadcast(&(pool->queue_not_full));

        pthread_mutex_unlock(&(pool->lock));

        // execuate task
        printf("thread 0x%d start working\n", (unsigned int)pthread_self());
        pthread_mutex_lock(&(pool->busy_thr_lock));
        pool->busy_thr_num++;
        pthread_mutex_unlock(&(pool->busy_thr_lock));
        (*(task.function))(task.args);

        // end task
        printf("thread 0x%x end working\n", (unsigned int)pthread_self());
        pthread_mutex_lock(&(pool->busy_thr_lock));
        pool->busy_thr_num--;
        pthread_mutex_unlock(&(pool->busy_thr_lock));
    }
    
    pthread_exit(NULL);

}

void* manage_thread(void* threadpool)
{
    int i = 0;
    threadpool_t* pool = (threadpool_t* )threadpool;
    while(!pool->shutdown)
    {
        sleep(DEFAULT_CHECK_TIME);

        pthread_mutex_lock(&(pool->lock));
        int queue_size = pool->queue_size;
        int live_thr_num = pool->live_thr_num;
        pthread_mutex_unlock(&(pool->lock));

        pthread_mutex_lock(&(pool->busy_thr_lock));
        int busy_thr_num = pool->busy_thr_num;
        pthread_mutex_unlock(&(pool->busy_thr_lock));

        if(queue_size >= MIN_WAIT_TASK_NUM && live_thr_num < pool->max_thr_num) {
            pthread_mutex_lock(&(pool->lock));
            int add_num = 0;

            for(int i = 0; i < pool->max_thr_num && add_num < DEFAULT_THREAD_VARY && pool->live_thr_num < pool->max_thr_num ; ++i) {
                printf("thread 0x%x manage_thread create thread\n", (unsigned int)pthread_self());
                
                if(pool->threads[i] == 0 || !is_live_thread(pool->threads[i])) {
                    pthread_create(&(pool->threads[i]), NULL, thread_work, (void*)pool);
                    add_num++;
                    pool->live_thr_num++;
                }
            }

            pthread_mutex_unlock(&(pool->lock));
            
        }

        if((busy_thr_num * 2) < live_thr_num && live_thr_num > pool->min_thr_num) {
            pthread_mutex_lock(&(pool->lock));
            pool->wait_exit_thr_num = DEFAULT_THREAD_VARY;
            pthread_mutex_unlock(&(pool->lock));

            for(i = 0; i < DEFAULT_THREAD_VARY; ++i) {
                pthread_cond_signal(&(pool->queue_not_empty));
            }
        }
    }

    return NULL;
}