#include<sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "protocol.hpp"
#include "tcp_recovery_client.hpp"
#include "packet_sequencer.hpp"
#include "market_data_applier.hpp"
#include "../Orderbook.hpp"

constexpr std::uint16_t Port = 5000;

int main(){
    const int socketFd = socket(AF_INET,SOCK_DGRAM,0);
    if(socketFd<0){
        std::cerr<<"Could not create UDP socket\n";
        return 1;
    }

    sockaddr_in receiverAddress{};
    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(Port);
    receiverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(socketFd, reinterpret_cast<sockaddr*>(&receiverAddress), sizeof(receiverAddress))<0 ){
        std::cerr<<"Could not bind UDP socket\n";
        close(socketFd);
        return 1;
    }


    std::uint32_t expected = 1;
    std::vector<PendingPacketSlot> packetBuffer(BUFFERSIZE);

    Orderbook book;
    MarketDataApplier applier(book);

    while(expected<=10){

        std::uint8_t buffer[MaxPacketLength]{};
        sockaddr_in senderAddress{};
        socklen_t senderAddressLength = sizeof(senderAddress);
        const ssize_t bytesReceived = recvfrom(
            socketFd,
            buffer,
            sizeof(buffer),
            0,
            reinterpret_cast<sockaddr*>(&senderAddress),
            &senderAddressLength
        );

        if(bytesReceived<0){
            std::cerr<<"Could not receive UDP message\n";
            close(socketFd);
            return 1;
        }

        const std::size_t receivedLength = static_cast<std::size_t> (bytesReceived);

        FeedPacket receivedPacket;
        if(!decodePacket(buffer,receivedLength, receivedPacket)){
            std::cerr<<"Packet corrupted. Could not decode\n";
            continue;
        }
        std::uint32_t received = receivedPacket.sequenceNumber;
        if(received == 0) {
            std::cerr<<"Invalid sequence number\n";
            continue;
        }
        if(received<expected){
            std::cout<<"Duplicate sequence detected. Discarding it\n";
            continue;
        }
        if(received - expected >= BUFFERSIZE) {
            std::cerr<<"Packet exceeds pending-buffer window; resync needed\n";
            close(socketFd);
            return 1;
        }
        std::size_t bufferIndex = (received-1) & (BUFFERSIZE-1);
        packetBuffer[bufferIndex] = PendingPacketSlot{received,true,receivedPacket};


        if(!drainPendingPackets(packetBuffer,expected,applier)){
                std::cout<<"Error processing the packets\n";
                close(socketFd);
                return 1;
            }

        if(expected<received){
            std::cout<<"Missing sequences from: "<<expected<<" to "<<received-1<<"\n";
            std::cout<<"Fetching missing packets from TCP server\n";
            std::vector<FeedPacket> recoveredPackets;
            if(!recoverMissingPackets(expected,received-1,recoveredPackets)){
                std::cerr<<"Could not receive missing packets\n";
                close(socketFd);
                return 1;
            }

            for(const auto& packet:recoveredPackets){
                const std::size_t bufferIndex = (packet.sequenceNumber-1) & (BUFFERSIZE-1);
                packetBuffer[bufferIndex] = PendingPacketSlot{packet.sequenceNumber,true,packet};
            }
            std::cout<<"Recovered Packets start\n";
            if(!drainPendingPackets(packetBuffer,expected,applier)){
                std::cout<<"Error processing the packets\n";
                close(socketFd);
                return 1;
            }
            std::cout<<"Recovered Packets end:\n";
        }
        
    }

    close(socketFd);
    return 0;

}