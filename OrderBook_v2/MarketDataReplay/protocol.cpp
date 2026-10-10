#include "protocol.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <limits>

namespace protocol{
    void pushUint64(std::vector<std::uint8_t> &buffer, std::uint64_t value){
        const std::uint32_t high = htonl(static_cast<std::uint32_t>(value>>32));
        const std::uint32_t low = htonl(static_cast<std::uint32_t>(value & 0xFFFFFFFF));
        const std::size_t oldSize = buffer.size();
        buffer.resize(oldSize + sizeof(value));

        std::memcpy(&buffer[oldSize],&high,sizeof(high));
        std::memcpy(&buffer[oldSize+sizeof(high)],&low,sizeof(low));
    }

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

    bool readUint64(const std::uint8_t* data, std::size_t length, std::size_t &offset, std::uint64_t& value){
        if(offset>length || length-offset < sizeof(std::uint64_t)){
            return false;
        }

        std::uint32_t high{},low{};
        std::memcpy(&high, data+offset, sizeof(high));
        std::memcpy(&low, data+offset+4, sizeof(low));
        high = ntohl(high);
        low = ntohl(low);
        value = (static_cast<std::uint64_t>(high)<<32) | (static_cast<std::uint64_t>(low));
        offset+= sizeof(value);
        return true;
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

std::vector<std::uint8_t> encodeAddOrderPayload( const AddOrderEvent& event){

    if(event.orderId==0 || event.price==0 || event.quantity==0){
        return {};
    }
    if(event.side!= FeedSide::Buy && event.side!=FeedSide::Sell){
        return {};
    }
    
    std::vector<std::uint8_t> buffer{};
    buffer.reserve(AddOrderPayloadSize);

    protocol::pushUint64(buffer,event.orderId);
    buffer.push_back(static_cast<std::uint8_t>(event.side));
    protocol::pushUint64(buffer, event.price);
    protocol::pushUint64(buffer, event.quantity);
    return buffer;
}

bool decodeAddOrderPayload(const std::vector<std::uint8_t>& payload,AddOrderEvent& event){
    std::size_t length = AddOrderPayloadSize;
    if(payload.size()!=length){
        return false;
    }
    const std::uint8_t *data = payload.data();

    if(data == nullptr){
        return false;
    }

    std::size_t offset = 0;
    if(!protocol::readUint64(data, length, offset, event.orderId)){
        return false;
    }
    if(event.orderId==0){
        return false;
    }

    const std::uint8_t side = data[offset];
    offset++;
    if(side!= static_cast<std::uint8_t>(FeedSide::Buy) && side!= static_cast<std::uint8_t>(FeedSide::Sell)){
        return false;
    }

    event.side = static_cast<FeedSide> (side);
    
    if(!protocol::readUint64(data,length,offset,event.price)){
        return false;
    }
    if(event.price==0){
        return false;
    }

    if(!protocol::readUint64(data, length, offset, event.quantity)){
        return false;
    }
    if(event.quantity==0){
        return false;
    }

    return true;
}

std::vector<std::uint8_t> encodeCancelOrderPayload( const CancelOrderEvent& event){

    if(event.orderId==0){
        return {};
    }
    std::vector<std::uint8_t> buffer{};
    buffer.reserve(CancelOrderPayloadSize);
    protocol::pushUint64(buffer,event.orderId);
    return buffer;
}


bool decodeCancelOrderPayload(const std::vector<std::uint8_t>& payload,CancelOrderEvent& event){
    std::size_t length = CancelOrderPayloadSize;
    if(payload.size()!=length){
        return false;
    }
    const std::uint8_t *data = payload.data();

    if(data == nullptr){
        return false;
    }

    std::size_t offset = 0;
    if(!protocol::readUint64(data, length, offset, event.orderId)){
        return false;
    }
    if(event.orderId==0){
        return false;
    }
    return true;
}
