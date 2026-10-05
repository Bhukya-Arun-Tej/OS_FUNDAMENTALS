#include<sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>

constexpr std::uint16_t Port = 5000;

int main(){
    const int socketFd = socket(PF_INET,SOCK_DGRAM,0);
    if(socketFd<0){
        std::cerr<<"Could not create UDP socket\n";
        return 1;
    }

    sockaddr_in receiverAddress{};
    receiverAddress.sin_family = PF_INET;
    receiverAddress.sin_port = htons(Port);
    receiverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(socketFd, reinterpret_cast<sockaddr*>(&receiverAddress), sizeof(receiverAddress))<0 ){
        std::cerr<<"Could not bind UDP socket\n";
        return 1;
    }

    char buffer[1024]{};
    sockaddr_in senderAddress{};
    socklen_t senderAddressLength = sizeof(senderAddress);

    const ssize_t bytesReceived = recvfrom(
        socketFd,
        buffer,
        sizeof(buffer)-1,
        0,
        reinterpret_cast<sockaddr*>(&senderAddress),
        &senderAddressLength
    );

    if(bytesReceived<0){
        std::cerr<<"Could not receive UDP message\n";
        return 1;
    }

    buffer[bytesReceived]='\0';
    std::cout<<"Received: "<<buffer<<" \n";
    close(socketFd);
    return 0;

}