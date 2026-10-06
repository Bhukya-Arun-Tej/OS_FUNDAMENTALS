#include "tcphelper.hpp"
#include <sys/types.h>
#include <sys/socket.h>


bool sendAll(int socketFd, const std::uint8_t* data, std::size_t length){
    std::size_t offset=0;

    while(offset<length){
        ssize_t bytesSent =  send(socketFd,data+offset,length-offset,0);
        if(bytesSent<=0){
            return false;
        }
        offset+= static_cast<std::size_t> (bytesSent);
    }
    return true;
}

bool receiveAll(int socketFd,std::uint8_t* data, std::size_t length){
    std::size_t offset=0;

    while(offset<length){
        ssize_t bytesReceived = recv(socketFd, data+offset, length-offset,0);
        if(bytesReceived<=0){
            return false;
        }
        offset += static_cast<std::size_t>(bytesReceived);
    }
    return true;

}