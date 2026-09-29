#pragma once
#include "TradeInfo.hpp"

class Trade{
private:
    TradeInfo m_buyTradeInfo{};
    TradeInfo m_sellTradeInfo{};
public:
    Trade() = default;
    Trade(TradeInfo buyTradeInfo, TradeInfo sellTradeInfo);
    TradeInfo getBuyTradeInfo() const;
    TradeInfo getSellTradeInfo() const;
};