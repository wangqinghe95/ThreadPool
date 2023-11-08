#include "threadpool.h"
#include <stdio.h>
#include <unistd.h>

void* process(void* arg)
{
    printf("therad 0x%x working on task %d\n", (unsigned int)pthread_self(), *(int*)arg);
    sleep(1);
    printf("task %d is end\n", *(int*)arg);

    return NULL;
}

int main()
{
    Thread_Pool_T* p_threadpool;
    p_threadpool = init_thread_pool(3,10,10);
    printf("Pool init\n");

    int num[20] = {0};
    for(int i = 0; i < 10; ++i) {
        num[i] = i;
        add_task_into_task_queue(p_threadpool, process, &num[i]);
    }

    sleep(5);

    destroy_thread_pool(p_threadpool);


    return 0;
}