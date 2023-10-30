# ThreadPool

## 需求
+ 实现一个线程池，要求如下
1. 线程池中最小3个线程，最大100个线程
2. 如果任务队列中等待执行的任务大于 10 时，开始增加 10 个线程
3. 如果线程池中空闲等待的线程大于 10 时，开始递减线程，至空闲线程数量降至 10 以下

## 实现
+ 一个线程池类，包含
    + 执行的工作线程，也是执行任务的函数
    + 保存任务的任务队列
    + 空闲工作线程锁，用来保证线程间互斥的获取任务队列中的任务
    + 执行任务线程锁，用来保证线程在工作时，需要对工作线程数量的操作
    + 执行任务队列满的条件变量，用来阻塞当前任务队列满了的时候的添加任务操作
    + 执行任务队列空的条件变量，用来阻塞当前任务队列空了的时候的执行任务操作

## Implement
+ [A versin of threadpool implemented by c](./version_c/README.md)
+ [A Thread Pool of C++ 11](./version_c++11/README.md)
