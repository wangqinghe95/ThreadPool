# ThreadPool

## Requirement
+ 用 C++11 实现一个线程池，要求：
    1. 可以传入任意类型函数，以及任意函数参数个数
    2. 支持线程异步，同步操作，也能通过同步获取传入函数的返回值，指定引用

## Introduction
+ [A ThreadPool implemented by C](./version_c/README.md)
+ [A ThreadPool implemented by C++11](./version_c++11/README.md)
+ [A WorkerThread from Android](./version_c++/utils/src/WorkThread/WorkerThread.cpp)

## [Implemented by C++11](./version_c++11/README.md)

### 任务队列类（TaskQueue）
1. 一个带有锁的队列，主要用来保持任务函数指针，并且队列元素在入队和出队过程中线程安全

### 线程池类（ThreadPool)
1. 初始化时，创建一定数量的线程池，阻塞在执行任务函数中，等待任务队列非空时获取任务执行
2. 任务添加函数，使用模板函数，通过可变函数参数列表支持接收不定函数参数的函数任务
    + 通过异步同步关键字支持查询等待提取异步的函数返回值
    + 通过 lambda 函数统一不同类型的函数，保持到任务队列中

### C++ 相关特性介绍
1. std::move
    + 将左值转换成右值引用，提取任务队列中任务时，转移队列中元素的资源所有权，避免重复构造实例
2. std::future
    + 异步编程关键字，可以查询，等待提取异步执行函数的结果
3. std::function
    + 绑定的执行任务接口
4. std::bind
    + 将任务函数和任务参数绑定一起，统一调用接口
6. std::forward
    + 完美转发，将外部传入的参数，保持其引用属性绑定到执行函数中
7. std::make_shared
    + 智能指针，无需考虑指针内存释放问题
8. std::lambda
    + 统一调用任务的接口

### TODO
+ 线程池管理
    1. 任务量多于线程数量时添加线程
    2. 空闲线程数量过多时销毁线程

## Reference
+ [知乎--C++11 实现一个线程池](https://zhuanlan.zhihu.com/p/367309864)