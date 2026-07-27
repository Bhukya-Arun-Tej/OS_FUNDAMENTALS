#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>


int main(){

    int sockfd;
    constexpr int yes=1, no=0;
    constexpr int MAXDATASIZE = 100;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3490);
    // addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    inet_pton(AF_INET,"127.0.0.1",&addr.sin_addr);


    sockfd = socket(AF_INET, SOCK_STREAM,0);
    if(sockfd==-1){
        perror("Socket create");
        return 1;
    }

    if(connect(sockfd,reinterpret_cast<sockaddr*>(&addr), sizeof(addr))==-1){
        perror("Connect");
        return 1;
    }
    
    std::cout<<"Starting client... \n";
    while(1){
        // sockaddr client;
        // socklen_t client_len;


        char buffer[MAXDATASIZE];
        std::cin.getline(buffer,sizeof(buffer));
        ssize_t bytesSent = send(sockfd, buffer, strlen(buffer), 0);
        if(bytesSent==-1){
            perror("Bytes sent");
            continue;
        }
        
        // std::cout<<">>> "<<buffer<<"\n";


        ssize_t bytesReceived = recv(sockfd, &buffer, MAXDATASIZE -1, 0);
        if(bytesReceived==-1){
            perror("Bytes received");
            continue;
        }
        buffer[bytesReceived]='\0';
        std::cout<<">>> "<<buffer<<"\n";
        close(sockfd);
        break;
    }

}

