#include<sys/socket.h>
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdint>
#include <vector>
#include "tcphelper.hpp"
#include "protocol.hpp"


constexpr std::uint16_t Port = 5001;
int main(){
    const int socketFd = socket(AF_INET,SOCK_STREAM,0);
    if(socketFd<0){
        std::cerr<<"Could not create TCP socket\n";
        return 1;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(Port);
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    

    if(connect(socketFd, reinterpret_cast<const sockaddr*>(&serverAddress),sizeof(serverAddress))<0){
        std::cerr<<"Could not connect to TCP server\n";
        close(socketFd);
        return 1;
    }
    std::cout<<"Connected to TCP recovery server\n";

    const std::uint32_t firstMissingSequenceNetwork = htonl(5);
    const std::uint32_t lastMissingSequenceNetwork = htonl(5);

    if(!sendAll(socketFd,reinterpret_cast<const std::uint8_t*>(&firstMissingSequenceNetwork), sizeof(firstMissingSequenceNetwork))){
        std::cerr<<"Error sending the firstMissingSequenceNetwork\n";
        close(socketFd);
        return 1;
    }
    if(!sendAll(socketFd,reinterpret_cast<const std::uint8_t*>(&lastMissingSequenceNetwork), sizeof(lastMissingSequenceNetwork))){
        std::cerr<<"Error sending the lastMissingSequenceNetwork\n";
        close(socketFd);
        return 1;
    }

    for(std::uint32_t sequence = 5; sequence<=5; sequence++){
            std::uint32_t packetLengthNetwork{};

            if(!receiveAll(socketFd, reinterpret_cast<std::uint8_t*> (&packetLengthNetwork), sizeof(packetLengthNetwork))){
                std::cerr<<"Error getting the requested packet length for the sequence- "<<sequence<<"\n";
                close(socketFd);
                return 1;
            }

            const std::uint32_t packetLength = ntohl(packetLengthNetwork);
            if(packetLength<PacketHeaderSize || packetLength>MaxPacketLength){
                std::cerr<<"Invalid recovery packet length";
                close(socketFd);
                return 1;
            }
            std::vector<std::uint8_t> encodedPacket(packetLength);
            if(!receiveAll(socketFd, encodedPacket.data(), packetLength)){
                std::cerr<<"Error getting the requested packet for sequence number - "<<sequence<<"\n";
                close(socketFd);
                return 1;
            }
            
            FeedPacket recoveredPacket{};
            if(!decodePacket(encodedPacket.data(),encodedPacket.size(),recoveredPacket)){
                std::cerr<<"Got corrupted packet from TCP recover server\n";
                close(socketFd);
                return 1;
            }

            if (recoveredPacket.sequenceNumber != sequence) {
                std::cerr << "Recovered an unexpected sequence number\n";
                close(socketFd);
                return 1;
            }

            std::cout<<"Payload of sequence - "<<sequence<<" is: "<<recoveredPacket.payload<<"\n";
        }


    close(socketFd);
    return 0;
}   