#include<sys/socket.h>
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdint>


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
    close(socketFd);
}   