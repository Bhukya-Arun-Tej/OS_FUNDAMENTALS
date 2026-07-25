#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>


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


        if(setsockopt(sockfd, SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))== -1){
            perror("Socket reuse");
            continue;
        }

        //bind socket
        if(bind(sockfd,ptr->ai_addr,ptr->ai_addrlen)==-1){
            perror("Bind");
            continue;
        }

        break;
    }

    freeaddrinfo(res);

    if(!ptr){
        std::cerr<<"Failed to bind the socket with an IP \n";
        return 1;
    }

    if(listen(sockfd,10)==-1){
        perror("Server Listen");
        return 1;
    }

    std::cout<<"Starting server... \n";
    while(1){
        sockaddr client;
        socklen_t client_len;
        int clientfd = accept(sockfd, &client, &client_len);
        if(clientfd==-1){
            perror("Accept");
            continue;
        }

        
        char buffer[MAXDATASIZE];
        ssize_t bytesReceived = recv(clientfd, &buffer, MAXDATASIZE -1, 0);
        if(bytesReceived==-1){
            perror("Bytes received");
            continue;
        }
        buffer[bytesReceived]='\0';
        std::cout<<"data received: "<<buffer<<"\n";

        strcpy(buffer, "The server is down for maintainance\n");

        ssize_t bytesSent = send(clientfd,&buffer,strlen(buffer),0);
        if(bytesSent==-1){
            perror("Bytes sent");
            continue;
        }

        close(clientfd);
        break;
    }

}

