#include "market_data_receiver.hpp"

MarketDataReceiver:: MarketDataReceiver(MarketDataApplier& applier): m_applier(applier), m_packetBuffer(BUFFERSIZE){};

std::uint32_t MarketDataReceiver::expectedSequence() const{
    return m_expected;
}

bool MarketDataReceiver::bufferPacket(const FeedPacket& packet){
    std::uint32_t received = packet.sequenceNumber;
    std::uint32_t expected = expectedSequence();
    if(received == 0) {
        return false;
    }
    if(received<expected){
        return false;
    }
    if(received - expected >= BUFFERSIZE) {
        return false;
    }
    std::size_t bufferIndex = (received-1) & (BUFFERSIZE-1);
    m_packetBuffer[bufferIndex] = PendingPacketSlot{received,true,packet};
    return true;
}

bool MarketDataReceiver:: drainPackets(){
    return drainPendingPackets(m_packetBuffer, m_expected ,m_applier);
}
