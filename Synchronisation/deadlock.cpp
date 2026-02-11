#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

std::mutex m1;
std::mutex m2;

void thread1() {
    std::cout << "Thread 1 locking m1\n";
    std::lock_guard<std::mutex> lock1(m1);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "Thread 1 trying to lock m2\n";
    std::lock_guard<std::mutex> lock2(m2);

    std::cout << "Thread 1 finished\n";
}

void thread2() {
    std::cout << "Thread 2 locking m2\n";
    std::lock_guard<std::mutex> lock1(m2);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "Thread 2 trying to lock m1\n";
    std::lock_guard<std::mutex> lock2(m1);

    std::cout << "Thread 2 finished\n";
}

int main() {
    std::thread t1(thread1);
    std::thread t2(thread2);

    t1.join();
    t2.join();
}
