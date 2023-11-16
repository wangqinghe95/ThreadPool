#include <iostream>
#include <unistd.h>
#include <string>
#include "ThreadPool.hpp"

void addFunc(int x, int y)
{

    std::this_thread::sleep_for(std::chrono::seconds(x));
    std::string msg =  std::to_string(x) + " + " + std::to_string(y) + " = " + std::to_string(x+y);

    std::cout << __func__ << ":" << __LINE__ << " " << msg << std::endl;
}

void subFunc(int x, int y)
{
    std::cout << __func__ << __LINE__ << std::endl;

    std::string msg = std::to_string(x) + " - " + std::to_string(y) + " = " + std::to_string(x-y);
    std::cout << msg << std::endl;
}

void multiply_output(int &out, const int a, const int b)
{
    std::this_thread::sleep_for(std::chrono::microseconds(1000));
    out = a * b;
    std::cout <<  a << " * " << b << " = " << out << std::endl;
}

int multiply_return(const int a, const int b)
{
    std::this_thread::sleep_for(std::chrono::microseconds(1000));
    int res = a * b;
    std::cout <<  a << " * " << b << " = " << res << std::endl;
    return res;   
}

int main()
{
    ThreadPool *p_ThreadPool = new ThreadPool(3,100,100);

    int output_ref;
    auto future1 = p_ThreadPool->submit(multiply_output, std::ref(output_ref), 5,6);
    future1.get();

    std::cout << "future1 out_ref: " << output_ref << std::endl;

    auto future2 = p_ThreadPool->submit(multiply_return, 2,4);
    int res = future2.get();
    std::cout << "future2 res: " << res << std::endl;

    for(int i = 1; i < 20; ++i) {
        p_ThreadPool->submit(addFunc, i, i+2);
    }

    std::cout << "waitting end..." << std::endl;

    sleep(20);

    p_ThreadPool->shutdown();
    delete p_ThreadPool;

    std::cout << "main thread end..." << std::endl;


    return 0;
}