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
#include "market_data_receiver.hpp"

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

    Orderbook book;
    MarketDataApplier applier(book);
    MarketDataReceiver receiver(applier);

    while(receiver.expectedSequence()<=10){

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
        if (!receiver.bufferPacket(receivedPacket)) {
            std::cerr << "Packet was duplicate, invalid, or outside the buffer window\n";
            continue;
        }
         if (!receiver.drainPackets()) {
            std::cerr << "Could not apply recovered packet\n";
            close(socketFd);
            return 1;
        }
       

        if(receiver.expectedSequence()<received){
            std::cout<<"Missing sequences from: "<<receiver.expectedSequence()<<" to "<<received-1<<"\n";
            std::cout<<"Fetching missing packets from TCP server\n";
            std::vector<FeedPacket> recoveredPackets;
            if(!recoverMissingPackets(receiver.expectedSequence(),received-1,recoveredPackets)){
                std::cerr<<"Could not receive missing packets\n";
                close(socketFd);
                return 1;
            }

            for(const auto& packet:recoveredPackets){
                receiver.bufferPacket(packet);
            }
            std::cout<<"Recovered Packets start\n";
            receiver.drainPackets();
            std::cout<<"Recovered Packets end:\n";
        }
        
    }

    book.printBuy();
    std::cout<<"\n";
    book.printSell();
    close(socketFd);
    return 0;

}