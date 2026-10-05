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

    

    for(std::uint32_t sequence = 1; sequence<=10; sequence++){
        if(sequence==5)continue;
        const std::string message = std::to_string(sequence);

        const ssize_t bytesSent = sendto(
            socketFd,
            message.data(),
            message.size(),
            0,
            reinterpret_cast<sockaddr*>(&receiverAddress),
            sizeof(receiverAddress)
        );

        if(bytesSent<0){
            std::cerr<<"Could not send UDP message\n";
            close(socketFd);
            return 1;
        }

        std::cout<<"Sent sequence: "<<sequence<<"\n";
        sleep(1);
    }
    close(socketFd);
    return 0;

}