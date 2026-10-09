#include "feed_producer.hpp"
#include "udp_publisher.hpp"
#include "protocol.hpp"
#include "feed_history.hpp"
#include <vector>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>


int runFeedProducer(FeedHistory& history){
    int udpSocketFd = socket(AF_INET,SOCK_DGRAM,0);
        if(udpSocketFd<0){
            std::cerr<<"UDP socket creation failure\n";
            return -1;
        }

    for(std::uint32_t sequence =1; sequence<=10;sequence++){
        std::vector<std::uint8_t> addOrderEvent = encodeAddOrderPayload(AddOrderEvent{
            sequence, FeedSide::Buy, 100 + sequence,sequence
        });
        FeedPacket packet{sequence, MessageType::AddOrder, addOrderEvent};
        const std::vector<std::uint8_t> encodedBytes = encodePacket(packet);

        if(encodedBytes.empty() || encodedBytes.size()<PacketHeaderSize){
            std::cerr<<"Encoding Failure\n";
            close(udpSocketFd);
            return -1;
        }

        history.store(sequence,encodedBytes);
        if(sequence == 5 || sequence == 6) {
            std::cout<<"Intentionally dropped UDP sequence: "<<sequence<<"\n";
            sleep(1);
            continue;
        }
        std::cerr<<"Sending the sequence: "<<sequence<<"\n";
        
        if(!publishBytesUDP(udpSocketFd,encodedBytes)){
            std::cerr<<"UDP publish failure\n";
            close(udpSocketFd);
            return -1;
        }
        sleep(1);
    }
    close(udpSocketFd);
    return 0;
}
