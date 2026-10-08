#pragma once

#include "protocol.hpp"
#include <vector>
#include <cstdint>


struct PendingPacketSlot {
    std::uint32_t sequenceNumber{};
    bool valid{false};
    FeedPacket packet;
};

inline constexpr std::uint32_t BUFFERSIZE = (1<<20);


void drainPendingPackets(std::vector<PendingPacketSlot>& packetBuffer,std::uint32_t& expected);
