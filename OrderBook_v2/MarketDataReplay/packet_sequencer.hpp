#pragma once

#include "protocol.hpp"
#include <vector>
#include <cstdint>
#include "market_data_applier.hpp"


struct PendingPacketSlot {
    std::uint32_t sequenceNumber{};
    bool valid{false};
    FeedPacket packet;
};

inline constexpr std::uint32_t BUFFERSIZE = (1<<20);


bool drainPendingPackets(std::vector<PendingPacketSlot>& packetBuffer,std::uint32_t& expected, MarketDataApplier& applier);
