# ThreadPool

## Requirement
+ 实现一个线程池，要求如下
    1. 线程池中最小3个线程，最大100个线程
    2. 如果任务队列中等待执行的任务大于 10 时，开始增加 10 个线程
    3. 如果线程池中空闲等待的线程大于 10 时，开始递减线程，至空闲线程数量降至 10 以下

## Implement
+ [A ThreadPool of C](./version_c/README.md)
+ [A ThreadPool of C++11](./version_c++11/README.md)

## Reference
+ [C++ 实现一个线程池](https://zhuanlan.zhihu.com/p/367309864)
