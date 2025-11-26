#include <iostream>
#include <thread>
#include <mach/mach_time.h>

// volatile is being added so that the compiler does not optimize the code, or else we cannot see the false sharing difference significantly
// volatile qualifier tells the compiler that it is sensitive to other processes and should not be optimised
struct hold{
  volatile long long val;
  volatile char padding[64 -sizeof(long long)];
};

hold x,y,a,b;

void work1(){
  for(int i=0;i<1000000000;i++){
    x.val++;
  }
}

void work2(){
  for(int i=0;i<1000000000;i++){
    y.val++;
  }
}

void work3(){
  for(int i=0;i<10000000;i++){
    a.val++;
  }
}

void work4(){
  for(int i=0;i<10000000;i++){
    b.val++;
  }
}
int main(){
  mach_timebase_info_data_t data;
  uint64_t start = mach_absolute_time();
  std::thread t1(work1);
  std:: thread t2(work2);
  std:: thread t3(work3);
  std:: thread t4(work4);
  t1.join();
  t2.join();
  t3.join();
  t4.join();
//   std::cout<<"x is "<<x.val<<"\ny is "<<y.val<<std::endl;
  uint64_t end = mach_absolute_time();
  mach_timebase_info(&data);
  uint64_t duration = end - start;
  std::cout<<duration*data.denom/data.numer<<std::endl;
  return 0;
}