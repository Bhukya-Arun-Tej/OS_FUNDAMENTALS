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
    receiverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    const char message[]  = "Hello from UDP publisher";

    const ssize_t bytesSent = sendto(
        socketFd,
        message,
        sizeof(message)-1,
        0,
        reinterpret_cast<sockaddr*>(&receiverAddress),
        sizeof(receiverAddress)
    );

    if(bytesSent<0){
        std::cerr<<"Could not send UDP message\n";
        close(socketFd);
        return 1;
    }

    std::cout<<"Sent: "<<bytesSent<<" bytes\n";
    close(socketFd);
    return 0;



}