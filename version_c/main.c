#include "threadpool.h"
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
    threadpool_t *thp = threadpool_create(3,100,100);
    printf("pool inited\n");

    int num[20];
    int i;

    sleep(1);
    for(i = 0; i < 20; i++) {
        num[i] = i;
        printf("add task %d\n", i);
        threadpool_add(thp, process, (void*)&num[i]);
    }

    sleep(100);
    threadpool_destory(thp);

    return 0;
}



