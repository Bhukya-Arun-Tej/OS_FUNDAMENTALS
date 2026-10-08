#include<sys/socket.h>
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdint>
#include <limits>
#include <vector>
#include "tcphelper.hpp"
#include "feed_history.hpp"
#include "tcp_recovery_server.hpp"


constexpr std::uint16_t Port = 5001;
constexpr int Limit = 10;
const int reuseAddress = 1;

bool publishBytesTCP(int clientFd, const std::vector<std::uint8_t> &encodedPacket){
    if(encodedPacket.empty() || encodedPacket.size()> std::numeric_limits<std::uint32_t>::max()){
        std::cerr<<"Invalud recovery packet size\n";
        return false;
    }

    const std::uint32_t packetLengthNetwork = htonl(static_cast<std::uint32_t>(encodedPacket.size()));

    if(!sendAll(clientFd, reinterpret_cast<const std::uint8_t*> (&packetLengthNetwork) , sizeof(packetLengthNetwork))){
        std::cerr<<"Error sending the requested packet length\n";
        return false;
    }

    if(!sendAll(clientFd, encodedPacket.data(), encodedPacket.size())){
        std::cerr<<"Error sending the requested packet\n";
        return false;
    }
    return true;
}


int runTcpRecoveryServer(const FeedHistory& history){
    const int socketFd = socket(AF_INET,SOCK_STREAM,0);
    if(socketFd<0){
        std::cerr<<"Could not create TCP socket\n";
        return 1;
    }

    if (setsockopt(socketFd,SOL_SOCKET,SO_REUSEADDR,&reuseAddress,sizeof(reuseAddress))<0){
        std::cerr << "Could not enable address reuse\n";
        close(socketFd);
        return 1;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(Port);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    


    if(bind(socketFd,reinterpret_cast<const sockaddr*> (&serverAddress),sizeof(serverAddress))<0){
        std::cerr<<"Could not bind the socket\n";
        close(socketFd);
        return 1;
    }

    if(listen(socketFd,Limit)<0){
        std::cerr<<"Could not listen\n";
        close(socketFd);
        return 1;
    }

    std::cout<<"Starting TCP server...\n";
    while(1){
        sockaddr_in clientAddress;
        socklen_t clientLength = sizeof(clientAddress);
        int clientFd = accept(socketFd,reinterpret_cast<sockaddr*>(&clientAddress),&clientLength);
        if(clientFd<0){
            std::cerr<<"Could not accept a connection\n";
            continue;
        }   
        std::cout<<"TCP recovery client connected\n";

        std::uint32_t firstMissingSequenceNetwork{};
        std::uint32_t lastMissingSequenceNetwork{};
        if(!receiveAll(clientFd, reinterpret_cast<std::uint8_t*> (&firstMissingSequenceNetwork), sizeof(firstMissingSequenceNetwork))){
            std::cerr<<"Could not receive the first missing sequence\n";
            close(clientFd);
            continue;
        }
        if(!receiveAll(clientFd, reinterpret_cast<std::uint8_t*> (&lastMissingSequenceNetwork), sizeof(lastMissingSequenceNetwork))){
            std::cerr<<"Could not receive the last missing sequence\n";
            close(clientFd);
            continue;
        }

        const std::uint32_t firstMissing = ntohl(firstMissingSequenceNetwork);
        const std::uint32_t lastMissing = ntohl(lastMissingSequenceNetwork);
        if (firstMissing==0 || firstMissing > lastMissing) {
            std::cerr << "Invalid recovery sequence range\n";
            close(clientFd);
            continue;
        }

        std::cout<<"Recover requested for sequence "<<firstMissing<<" through "<<lastMissing<<"\n"; 

        for(std::uint32_t sequence = firstMissing; sequence<=lastMissing; sequence++){
            
            std::vector<std::uint8_t> encodedPacket;
            
            //fetch the encoded bytes from recovery history
            if(!history.find(sequence,encodedPacket)){
                std::cerr<<"Feed history not found\n";
                break;
            }


            if(!publishBytesTCP(clientFd, encodedPacket)){
                break;
            }
            
        }
        
        close(clientFd);
    }

    return 0;

}   