#include "threadpool.h"

int main()
{
    Thread_Pool_T* p_threadpool;
    p_threadpool = init_thread_pool(3,10,10);

    destroy_thread_pool(p_threadpool);

    return 0;
}