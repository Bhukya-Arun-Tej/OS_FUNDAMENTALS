#include<iostream>

using Capacity = uint64_t;
using Size = uint64_t;
using Data = uint64_t;

template <typename T>
class vector{
    private:
    Capacity capacity;
    Size size;
    uint64_t *localarray;

    public:
    vector(){
        this->capacity = 16;
        localarray = new uint64_t(capacity);
    }
    vector(Size size){
        int power = 0;
        while(size){
            size/=2;
            power++;
        }
        this->capacity = (1<<power);
        this->size = size;
        localarray = new uint64_t(capacity);
    }

    Capacity getCapacity() const{
        return capacity;
    }

    void push_back(Data data){
        
    }

};  


int main(){
    vector<int> temp(10);
    std::cout<<temp.getCapacity()<<std::endl;
    std::cout<<temp.getArraySize()<<std::endl;
}