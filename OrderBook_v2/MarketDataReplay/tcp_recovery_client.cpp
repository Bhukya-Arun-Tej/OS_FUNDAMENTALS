#include<sys/socket.h>
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdint>
#include <vector>
#include "tcphelper.hpp"
#include "protocol.hpp"
#include "tcp_recovery_client.hpp"

constexpr std::uint16_t Port = 5001;

bool recoverMissingPackets(std::uint32_t firstMissingSequence, std::uint32_t lastMissingSequence, std::vector<FeedPacket>& recoveredPackets){
    if(firstMissingSequence>lastMissingSequence){
        return false;
    }
    
    const int socketFd = socket(AF_INET,SOCK_STREAM,0);
    if(socketFd<0){
        std::cerr<<"Could not create TCP socket\n";
        return false;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(Port);
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    

    if(connect(socketFd, reinterpret_cast<const sockaddr*>(&serverAddress),sizeof(serverAddress))<0){
        std::cerr<<"Could not connect to TCP server\n";
        close(socketFd);
        return false;
    }
    std::cout<<"Connected to TCP recovery server\n";

    const std::uint32_t firstMissingSequenceNetwork = htonl(firstMissingSequence);
    const std::uint32_t lastMissingSequenceNetwork = htonl(lastMissingSequence);

    if(!sendAll(socketFd,reinterpret_cast<const std::uint8_t*>(&firstMissingSequenceNetwork), sizeof(firstMissingSequenceNetwork))){
        std::cerr<<"Error sending the firstMissingSequenceNetwork\n";
        close(socketFd);
        return false;
    }
    if(!sendAll(socketFd,reinterpret_cast<const std::uint8_t*>(&lastMissingSequenceNetwork), sizeof(lastMissingSequenceNetwork))){
        std::cerr<<"Error sending the lastMissingSequenceNetwork\n";
        close(socketFd);
        return false;
    }

    for(std::uint32_t sequence = firstMissingSequence; sequence<= lastMissingSequence; sequence++){
            std::uint32_t packetLengthNetwork{};

            if(!receiveAll(socketFd, reinterpret_cast<std::uint8_t*> (&packetLengthNetwork), sizeof(packetLengthNetwork))){
                std::cerr<<"Error getting the requested packet length for the sequence- "<<sequence<<"\n";
                close(socketFd);
                return false;
            }

            const std::uint32_t packetLength = ntohl(packetLengthNetwork);
            if(packetLength<PacketHeaderSize || packetLength>MaxPacketLength){
                std::cerr<<"Invalid recovery packet length";
                close(socketFd);
                return false;
            }
            std::vector<std::uint8_t> encodedPacket(packetLength);
            if(!receiveAll(socketFd, encodedPacket.data(), packetLength)){
                std::cerr<<"Error getting the requested packet for sequence number - "<<sequence<<"\n";
                close(socketFd);
                return false;
            }
            
            FeedPacket recoveredPacket{};
            if(!decodePacket(encodedPacket.data(),encodedPacket.size(),recoveredPacket)){
                std::cerr<<"Got corrupted packet from TCP recover server\n";
                close(socketFd);
                return false;
            }

            if (recoveredPacket.sequenceNumber != sequence) {
                std::cerr << "Recovered an unexpected sequence number\n";
                close(socketFd);
                return false;
            }

            recoveredPackets.push_back(recoveredPacket);
        }
    close(socketFd);
    return true;
}
   