#include <iostream>
#include <thread>
#include <atomic>
#include <mach/mach_time.h>

class Node{
    public:
    Node* next;
    int data;

    Node(int val){
        this->data = val;
        next = nullptr;
    }

};

class nspsc{
    private:
        std:: atomic<Node*> head;
        Node* tail;

    public:
        nspsc(){
            Node* dummy = new Node(0);
            head.store(dummy, std:: memory_order_relaxed);
            tail = dummy;
        }

        void push(int data){
            Node* ptr = new Node(data);
            tail->next = ptr;
            tail = ptr;
        }

        bool pop(int &v){
            Node* first= head.load(std::memory_order_relaxed);
            Node* next = first->next;
            if(next == nullptr){
                return false;
            }

            v = next->data;
            head.store(next, std::memory_order_relaxed);
            delete first;
            return true;

        }

};

nspsc q;


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