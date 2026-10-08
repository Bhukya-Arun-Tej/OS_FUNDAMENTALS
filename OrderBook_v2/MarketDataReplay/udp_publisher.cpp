#include<sys/socket.h>
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "udp_publisher.hpp"

constexpr std::uint16_t Port = 5000;

bool publishBytesUDP(int socketFd, const std::vector<std::uint8_t>  &encodedBytes){
    sockaddr_in receiverAddress{};
    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(Port);
    receiverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    const ssize_t bytesSent = sendto(
            socketFd,
            encodedBytes.data(),
            encodedBytes.size(),
            0,
            reinterpret_cast<const sockaddr*>(&receiverAddress),
            sizeof(receiverAddress)
        );

        if(bytesSent<0){
            std::cerr<<"Could not send UDP encoded bytes\n";
            return false;
        }

        if (static_cast<std::size_t>(bytesSent) != encodedBytes.size()) {
            std::cerr << "UDP datagram was not fully sent\n";
            return false;
        }

        return true;
}


