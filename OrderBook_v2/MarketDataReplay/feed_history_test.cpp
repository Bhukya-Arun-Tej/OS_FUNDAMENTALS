#include "feed_history.hpp"
#include <stdexcept>
#include <cstdint>
#include <iostream>
#include <vector>

class FeedHistoryTest{
    public: 
        bool runAllTests(){
            bool allPassed = true;
            if(rejectZeroCapacity()){
                std::cout<<"PASS: rejectZeroCapacity\n";
            }
            else{
                std::cout<<"FAIL: rejectZeroCapacity\n";
                allPassed=0;
            }

            if(rejectNonPowerOfTwoCapacity()){
                std::cout<<"PASS: rejectNonPowerOfTwoCapacity\n";
            }
            else{
                std::cout<<"FAIL: rejectNonPowerOfTwoCapacity\n";
                allPassed=0;
            }

            if(storesAndFindsPacket()){
                std::cout<<"PASS: storesAndFindsPacket\n";
            }
            else{
                std::cout<<"FAIL: storesAndFindsPacket\n";
                allPassed=0;
            }

            if(doesNotFindUnknownPacket()){
                std::cout<<"PASS: doesNotFindUnknownPacket\n";
            }
            else{
                std::cout<<"FAIL: doesNotFindUnknownPacket\n";
                allPassed=0;
            }

            if(overwritesOldPacketWhenCapacityIsReached()){
                std::cout<<"PASS: overwritesOldPacketWhenCapacityIsReached\n";
            }
            else{
                std::cout<<"FAIL: overwritesOldPacketWhenCapacityIsReached\n";
                allPassed=0;
            }

            return allPassed;
        }

    private:
        bool rejectZeroCapacity(){
            try{
                FeedHistory FeedHistory(0);
            }
            catch(const std::exception& e){
                return 1;
            }
            return 0;
        }

        bool rejectNonPowerOfTwoCapacity(){
            try{
                FeedHistory history(3);
            }
            catch(const std::exception& e){
                return 1;
            }
            return 0;
        }

        bool storesAndFindsPacket(){
            FeedHistory history(4);
            const std::vector<std::uint8_t> bytes{1,2,3,4};
            std::vector<std::uint8_t> result;

            history.store(1, bytes);

            return history.find(1, result) && result == bytes;
        }

        bool doesNotFindUnknownPacket(){
            FeedHistory history(4);
            std::vector<std::uint8_t> result;

            return !history.find(1, result);
        }

        bool overwritesOldPacketWhenCapacityIsReached(){
            FeedHistory history(4);
            const std::vector<std::uint8_t> firstBytes{1};
            const std::vector<std::uint8_t> secondBytes{2};
            const std::vector<std::uint8_t> thirdBytes{3};
            const std::vector<std::uint8_t> fourthBytes{4};
            const std::vector<std::uint8_t> fifthBytes{5};
            std::vector<std::uint8_t> result;

            history.store(1, firstBytes);
            history.store(2, secondBytes);
            history.store(3, thirdBytes);
            history.store(4, fourthBytes);
            history.store(5, fifthBytes);

            return !history.find(1, result) &&
                   history.find(5, result) &&
                   result == fifthBytes;
        }
};


int main(){
    FeedHistoryTest feedHistoryTest;
    return feedHistoryTest.runAllTests() ? 0 : 1;
}
