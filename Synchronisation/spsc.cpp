#include<iostream>
#include<atomic>
#include<thread>
#include <mach/mach_time.h>
#include <spsc.hpp>

spsc<int, 1024> q;

void producer(){
    int val = rand()%100000+1;
    q.push(val);
}

int consumer(){
    int val ;
    q.pop(val);
    return val&1;
}


int main(){
    mach_timebase_info_data_t time;
    uint64_t start = mach_absolute_time();


    std::thread t1(producer);
    std:: thread t2(consumer);

    t1.join();
    t2.join();

    uint64_t end = mach_absolute_time();
    mach_timebase_info(&time);
    uint64_t duration = end - start; 
    std::cout<<duration*time.numer/time.denom<<std::endl;


    return 0;

}