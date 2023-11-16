# ThreadPool

## Requirement
+ 用 C++11 实现一个线程池，要求：
    1. 可以传入任意类型函数，以及任意函数参数个数
    2. 支持线程异步，同步操作，也能通过同步获取传入函数的返回值，指定引用

## Introduction
+ [A ThreadPool implemented by C](./version_c/README.md)
+ [A ThreadPool implemented by C++11](./version_c++11/)
+ [A WorkerThread from Android](./version_c++/utils/src/WorkThread/WorkerThread.cpp)

## [Implemented by C++11](./version_c++11/README.md)

### 任务队列类（TaskQueue）
1. 一个带有锁的队列，主要用来保持任务函数指针，并且队列元素在入队和出队过程中线程安全

### 线程池类（ThreadPool)
1. 初始化时，创建一定数量的线程池，阻塞在执行任务函数中，等待任务队列非空时获取任务执行
2. 任务添加函数，使用模板函数，通过可变函数参数列表支持接收不定函数参数的函数任务
    + 通过异步同步关键字支持查询等待提取异步的函数返回值
    + 通过 lambda 函数统一不同类型的函数，保持到任务队列中

## Reference
+ [知乎--C++11 实现一个线程池](https://zhuanlan.zhihu.com/p/367309864)