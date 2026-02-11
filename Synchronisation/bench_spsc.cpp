#include <boost/lockfree/spsc_queue.hpp>
#include <chrono>
#include <iostream>
#include <thread>
#include "spsc.hpp"

constexpr size_t N = 10'000'000;
constexpr size_t CAP = 1024;

void bench_ring() {
    spsc<int, CAP> q;

    auto start = std::chrono::high_resolution_clock::now();

    std::thread prod([&] {
        for (size_t i = 0; i < N; ) {
            if (q.push(i)) i++;
        }
    });

    std::thread cons([&] {
        int v;
        for (size_t i = 0; i < N; ) {
            if (q.pop(v)) i++;
        }
    });

    prod.join();
    cons.join();

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Ring buffer: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count()
              << " ms\n";
    std::cout<<"size is "<<q.size()<<std::endl;
}

void bench_boost() {
    boost::lockfree::spsc_queue<int,
        boost::lockfree::capacity<CAP>> q;

    auto start = std::chrono::high_resolution_clock::now();

    std::thread prod([&] {
        for (size_t i = 0; i < N; ) {
            if (q.push(i)) i++;
        }
    });

    std::thread cons([&] {
        int v;
        for (size_t i = 0; i < N; ) {
            if (q.pop(v)) i++;
        }
    });

    prod.join();
    cons.join();

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Boost SPSC: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count()
              << " ms\n";
}

int main() {
    bench_ring();
    bench_boost();
}
