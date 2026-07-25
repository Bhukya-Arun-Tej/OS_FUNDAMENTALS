#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>


int main(){
    addrinfo hints, *res, *ptr;
    int sockfd, yes=1, no=0;
    int MAXDATASIZE = 100;
    std::string port = "3490";
    

    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;

    if(getaddrinfo(NULL, port.c_str(),&hints,&res)){
        perror("getaddrinfo");
        return 2;
    }

    for(ptr=res;ptr!=NULL; ptr=ptr->ai_next){
        //create socket
        sockfd = socket(ptr->ai_family, ptr->ai_socktype,ptr->ai_protocol);
        if(sockfd==-1){
            perror("Socket create");
            continue;
        }

        if(connect(sockfd,ptr->ai_addr, ptr->ai_addrlen)==-1){
            perror("Connect");
            continue;
        }

        break;
    }

    freeaddrinfo(res);

    

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

