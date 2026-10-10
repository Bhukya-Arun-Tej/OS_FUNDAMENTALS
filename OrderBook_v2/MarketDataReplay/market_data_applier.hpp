#pragma once

#include "protocol.hpp"
#include "../Orderbook.hpp"

class MarketDataApplier{
    public: 
        MarketDataApplier(Orderbook& book);
        bool processPacket(const FeedPacket& packet);
    private:
        Orderbook& m_book;
};