#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>


int main(){
    int sockfd;
    constexpr int yes=1, no=0;
    constexpr int MAXDATASIZE = 100;


    sockaddr_in addr{};   
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3490);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd==-1){
        perror("Socket create");
        return 1;
    }

    if(setsockopt(sockfd, SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))== -1){
        perror("Socket reuse");
        return 1;
    }

    if(bind(sockfd,reinterpret_cast<sockaddr*>(&addr),sizeof(addr))==-1){
        perror("Bind");
        return 1;
    }

    if(listen(sockfd,10)==-1){
        perror("Server Listen");
        return 1;
    }

    std::cout<<"Starting server... \n";
    while(1){
        sockaddr client;
        socklen_t client_len = sizeof(client);
        int clientfd = accept(sockfd, &client, &client_len);
        if(clientfd==-1){
            perror("Accept");
            continue;
        }

        
        char buffer[MAXDATASIZE];
        ssize_t bytesReceived = recv(clientfd, buffer, MAXDATASIZE -1, 0);
        if(bytesReceived==-1){
            perror("Bytes received");
            continue;
        }
        buffer[bytesReceived]='\0';
        std::cout<<"data received: "<<buffer<<"\n";

        strcpy(buffer, "The server is down for maintainance\n");

        ssize_t bytesSent = send(clientfd,buffer,strlen(buffer),0);
        if(bytesSent==-1){
            perror("Bytes sent");
            continue;
        }

        close(clientfd);
        break;
    }

}

