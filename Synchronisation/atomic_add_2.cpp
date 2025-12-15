#include <iostream>
#include <thread>
#include <atomic>
#include <mach/mach_time.h>

std::atomic<int> counter{0};

void add(int id){
    int local =0;
    for(int i=0;i<10000;i++){
        local++;
    }
    counter.fetch_add(local);
}


int main(){
     mach_timebase_info_data_t data;
    uint64_t start = mach_absolute_time();

    std::thread t1(add,1);
    std::thread t2(add,2);
    t1.join();
    t2.join();
    uint64_t end = mach_absolute_time();
    mach_timebase_info(&data);
    uint64_t duration = end - start; 
    std::cout<<counter.load()<<std::endl;
    std::cout<<duration*data.numer/data.denom<<std::endl;

    return 0;
}