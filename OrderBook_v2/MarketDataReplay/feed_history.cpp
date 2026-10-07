#include "feed_history.hpp"
#include <vector>
#include <cstddef>          
#include <cstdint>
#include <stdexcept>
#include <bit>

FeedHistory::FeedHistory(std::size_t capacity) : m_capacity{capacity}{
    if(capacity==0 || std::popcount(capacity)!=1){
        throw std::invalid_argument("Feed history capacity must be a power of 2");
    }
    m_history.resize(capacity,{});
}

void FeedHistory::store(std::uint32_t sequenceNumber, const std::vector<std::uint8_t>& bytes){
    if (sequenceNumber == 0) {
      throw std::invalid_argument("Sequence number must be positive");
    }
    const std::size_t index = (sequenceNumber-1) & (m_capacity-1);
    m_history[index] = {sequenceNumber, bytes}; 
}

bool FeedHistory::find(std::uint32_t sequenceNumber, std::vector<std::uint8_t>& result)const{
    if (sequenceNumber == 0) {
        return false;
    }
    const std::size_t index = (sequenceNumber-1) & (m_capacity-1);
    const StoredPacket &packet = m_history[index];
    if(packet.sequenceNumber != sequenceNumber){
        return false;
    }
    result = packet.bytes;
    return true;
}


