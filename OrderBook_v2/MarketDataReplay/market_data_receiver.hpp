#pragma once
#include "market_data_applier.hpp"
#include "packet_sequencer.hpp"

class MarketDataReceiver{
    private:
        MarketDataApplier& m_applier;
        std::uint32_t m_expected{1};
        std::vector<PendingPacketSlot> m_packetBuffer;

    public:
        explicit MarketDataReceiver(MarketDataApplier& appiler);
        bool bufferPacket(const FeedPacket & packet);
        bool drainPackets();
        std::uint32_t expectedSequence() const;
};