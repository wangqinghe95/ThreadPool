#include <iostream>
#include <string>
#include <chrono>

#include "WorkerThread.hpp"


using std::string;
using std::cout;
using std::endl;

std::chrono::microseconds durations(10);

void add(int a, int b) {
    string str_print = std::to_string(a) + " + " + std::to_string(b) + " = " + std::to_string(a+b);
    cout << str_print << endl;
}

int main()
{
    WorkerThread work_thread;
    int a = 10;
    int b = 20;
    auto task = [a,b]() {
        add(a,b);
    };

    work_thread.schedule(task, durations);


    // std::this_thread::sleep_for(std::chrono::seconds(3));

    // work_thread.cancelAll();

    cout << "exit" << endl;

    return 0;
}