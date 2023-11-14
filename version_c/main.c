#include "threadpool.h"
#include <stdio.h>
#include <unistd.h>

void* process(void* arg)
{
    sleep(*(int*)arg);
    printf("after sleep %d s, task %d is end\n",*(int*)arg, *(int*)arg);

    return NULL;
}

int main()
{
    Thread_Pool_T* p_threadpool;
    p_threadpool = initThreadPool(3,30,10);
    printf("Pool init\n");

    int num[40] = {0};
    for(int i = 0; i < 40; ++i) {
        num[i] = i;
        addTask2Queue(p_threadpool, process, &num[i]);
    }
    
    printf("Main add task end, wait exited\n");

    sleep(200);

    printf("Main execuate 200s, and exit\n");
    destroyThreadPool(p_threadpool);
    return 0;
}