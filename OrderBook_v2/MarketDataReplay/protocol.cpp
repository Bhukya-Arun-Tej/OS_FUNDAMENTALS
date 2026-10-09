#include "protocol.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <limits>

namespace protocol{
    void pushUint32(std::vector<std::uint8_t> &buffer, std::uint32_t value){
        const std::uint32_t newValue = htonl(value);
        const std::size_t oldSize = buffer.size();
        buffer.resize(oldSize + sizeof(newValue));

        std::memcpy(&buffer[oldSize],&newValue,sizeof(newValue));
    }

    void pushUint16(std::vector<std::uint8_t> &buffer, std::uint16_t value){
        const std::uint16_t newValue = htons(value);
        const std::size_t oldSize = buffer.size();
        buffer.resize(oldSize + sizeof(newValue));

        std::memcpy(&buffer[oldSize],&newValue,sizeof(newValue));
    }

    bool readUint32(const std::uint8_t* data, std::size_t length, std::size_t &offset, std::uint32_t& value){
        if(offset>length || length-offset < sizeof(std::uint32_t)){
            return false;
        }

        std::uint32_t networkValue{};
        std::memcpy(&networkValue, data+offset, sizeof(networkValue));
        value = ntohl(networkValue);
        offset+= sizeof(networkValue);
        return true;
    }

    bool readUint16(const std::uint8_t* data, std::size_t length, std::size_t &offset, std::uint16_t& value){
        if(offset>length || length-offset < sizeof(std::uint16_t)){
            return false;
        }

        std::uint16_t networkValue{};
        std::memcpy(&networkValue, data+offset, sizeof(networkValue));
        value = ntohs(networkValue);
        offset+= sizeof(networkValue);
        return true;
    }
}


std::vector<std::uint8_t> encodePacket(const FeedPacket& packet){

    if(packet.payload.size() > std::numeric_limits<PayloadLength>::max()){
        return {};
    }
    if (packet.sequenceNumber == 0 || packet.payload.size()>MaxPacketLength-PacketHeaderSize) {
        return {};
    }
    if (packet.messageType != MessageType::Test && packet.messageType != MessageType::AddOrder && packet.messageType != MessageType::CancelOrder ) {
        return {};
    }
    std::vector<std::uint8_t> buffer{};
    buffer.reserve(PacketHeaderSize+packet.payload.size());
    protocol::pushUint32(buffer,FeedMagic);
    protocol::pushUint16(buffer, FeedVersion);
    protocol::pushUint16(buffer, static_cast<std::uint16_t>(packet.messageType));
    protocol::pushUint32(buffer, packet.sequenceNumber);
    protocol::pushUint16(buffer, static_cast<PayloadLength>(packet.payload.size()));
    buffer.insert(buffer.end(),packet.payload.begin(),packet.payload.end());

    return buffer;
}

bool decodePacket(const std::uint8_t* data, std::size_t length, FeedPacket &packet){
    if(length < PacketHeaderSize || data == nullptr){
        return false;
    }

    std::size_t offset = 0;
    std::uint32_t feedMagic{};
    if(!protocol::readUint32(data, length, offset, feedMagic)){
        return false;
    }
    if(feedMagic != FeedMagic){
        return false;
    }

    std::uint16_t feedVersion{};
    if(!protocol::readUint16(data,length,offset,feedVersion)){
        return false;
    }
    if(feedVersion != FeedVersion){
        return false;
    }

    std::uint16_t messageType{};
    if(!protocol::readUint16(data,length,offset,messageType)){
        return false;
    }

    if (messageType != static_cast<std::uint16_t>(MessageType::Test) &&
        messageType != static_cast<std::uint16_t>(MessageType::AddOrder) &&
        messageType != static_cast<std::uint16_t>(MessageType::CancelOrder)) {
        return false;
    }

    std::uint32_t sequenceNumber{};
    if(!protocol::readUint32(data, length, offset, sequenceNumber)){
        return false;
    }
    if(sequenceNumber==0){
        return false;
    }

    PayloadLength payloadSize{};
    if(!protocol::readUint16(data, length, offset, payloadSize)){
        return false;
    }

    if(length !=payloadSize + offset){
        return false;
    }

    packet.messageType = static_cast<MessageType>(messageType);
    packet.sequenceNumber = sequenceNumber;
    packet.payload.assign(data+offset, data+offset+payloadSize);

    return true;
}
