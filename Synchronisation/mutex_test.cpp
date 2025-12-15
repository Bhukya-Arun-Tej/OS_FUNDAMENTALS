#include<iostream>
#include<thread>
#include <mutex>
#include <mach/mach_time.h>


int counter=0;
std::mutex m;

void add (int id){
    m.lock();
    for(int i=0;i<10000;i++){
        counter++;
    }
    m.unlock();
}



int main(){

    mach_timebase_info_data_t data;
    uint64_t start = mach_absolute_time();
    
    std::thread t1(add,1);
    std::thread t2(add,2);
    t1.join();
    t2.join();
    std::cout<<counter<<std::endl;
    
    uint64_t end = mach_absolute_time();
    mach_timebase_info(&data);
    uint64_t duration = end - start; 
    std::cout<<duration*data.numer/data.denom<<std::endl;
    return 0;
}