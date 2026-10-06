#include<sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "protocol.hpp"
#include "tcp_recovery_client.hpp"

constexpr std::uint16_t Port = 5000;

int main(){
    const int socketFd = socket(AF_INET,SOCK_DGRAM,0);
    if(socketFd<0){
        std::cerr<<"Could not create UDP socket\n";
        return 1;
    }

    sockaddr_in receiverAddress{};
    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(Port);
    receiverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(socketFd, reinterpret_cast<sockaddr*>(&receiverAddress), sizeof(receiverAddress))<0 ){
        std::cerr<<"Could not bind UDP socket\n";
        return 1;
    }


    std::uint32_t expected = 1;

    while(expected<=10){

        std::uint8_t buffer[1024]{};
        sockaddr_in senderAddress{};
        socklen_t senderAddressLength = sizeof(senderAddress);
        const ssize_t bytesReceived = recvfrom(
            socketFd,
            buffer,
            sizeof(buffer),
            0,
            reinterpret_cast<sockaddr*>(&senderAddress),
            &senderAddressLength
        );

        if(bytesReceived<0){
            std::cerr<<"Could not receive UDP message\n";
            return 1;
        }

        const std::size_t receivedLength = static_cast<std::size_t> (bytesReceived);

        FeedPacket receivedPacket;
        if(!decodePacket(buffer,receivedLength, receivedPacket)){
            std::cerr<<"Packet corrupted. Could not decode\n";
            continue;
        }
        std::uint32_t received = receivedPacket.sequenceNumber;

        if(received==expected){
            std::cout<<"Received sequence: "<<received<<" \n";
            ++expected;
        }
        else if(received> expected){
            std::cout<<"Missing sequences from: "<<expected<<" to "<<received-1<<"\n";
            std::cout<<"Fetching missing packets from TCP server\n";
            std::vector<FeedPacket> recoveredPackets;
            if(!recoverMissingPackets(expected,received-1,recoveredPackets)){
                std::cerr<<"Could not receive missing packets\n";
                return 1;
            }
            for(const auto& packet:recoveredPackets){
                std::cout<<"Received sequence: "<<packet.sequenceNumber<<"\n";
            }
            std::cout<<"Received sequence: "<<received<<" \n";

            expected = received+1;
        }
        else{
            std::cout<<"Duplicate sequence: "<<received<<". Dropping packet\n";
        }
    }

    close(socketFd);
    return 0;

}