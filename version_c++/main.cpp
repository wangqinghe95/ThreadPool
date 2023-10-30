#include <iostream>
#include <unistd.h>
#include <string>
#include "ThreadPool.hpp"

void addFunc(int x, int y)
{
    std::cout << __func__ << __LINE__ << std::endl;

    std::string msg = std::to_string(x) + " + " + std::to_string(y) + " = " + std::to_string(x+y);
    std::cout << msg << std::endl;
}

void subFunc(int x, int y)
{
    std::cout << __func__ << __LINE__ << std::endl;

    std::string msg = std::to_string(x) + " - " + std::to_string(y) + " = " + std::to_string(x-y);
    std::cout << msg << std::endl;
}

int main()
{
    ThreadPool *p_ThreadPool = new ThreadPool(3,100,100);

    auto f_add = std::bind(&addFunc, 1, 2);
    p_ThreadPool->addTask(f_add);

    auto f_sub = std::bind(&subFunc, 1, 2);
    p_ThreadPool->addTask(f_sub);

    sleep(10);

    p_ThreadPool->shutdown();
    delete p_ThreadPool;

    return 0;
}