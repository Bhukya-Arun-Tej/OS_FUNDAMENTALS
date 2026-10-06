#include<sys/socket.h>
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdint>


constexpr std::uint16_t Port = 5001;
constexpr int Limit = 10;
const int reuseAddress = 1;
int main(){
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
        sockaddr clientAddress;
        socklen_t clientLength = sizeof(clientAddress);
        int clientFd = accept(socketFd,&clientAddress,&clientLength);
        if(clientFd<0){
            std::cerr<<"Could not accept a connection\n";
            continue;
        }   

        std::cout<<"TCP recovery client connected\n";
        close(clientFd);
    }


}   