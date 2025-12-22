#include<iostream>
#include<atomic>
#include<thread>
#include <mach/mach_time.h>

template<typename T , size_t Capacity>
class spsc{
    static_assert((Capacity & Capacity-1) ==0, "Capacity must be a power of 2");
    private:
        alignas(64) std:: atomic<size_t> head{0};
        alignas(64) std:: atomic<size_t> tail{0};
        T buffer[Capacity];

    public:
        bool push(const T& data){
            size_t t = tail.load(std::memory_order_relaxed);
            size_t next = (t+1) & (Capacity-1);
            if(next == head.load(std::memory_order_relaxed)){
                return false;
            }

            buffer[t] = data;
            tail.store(next, std::memory_order_relaxed);
            return true;
        }

        bool pop(T& data){
            size_t h = head.load(std::memory_order_relaxed);
            if(h == tail.load(std::memory_order_relaxed)){
                return false;
            }

            data = buffer[h];
            head.store((h+1) & (Capacity-1), std::memory_order_relaxed);
            return true;
        }
    };
