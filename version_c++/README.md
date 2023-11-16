# A ThreadPool implemented by C++11

## Introduction

### Compiler
`make && ./threadpool`

### files
1. main.cpp
    + test file
2. ThreadPool.hpp
    + class TaskQueue
    + class ThreadPool

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