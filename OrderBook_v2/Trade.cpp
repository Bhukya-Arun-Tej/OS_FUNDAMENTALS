#include "Trade.hpp"
#include <map>


Trade::Trade(TradeInfo buyTradeInfo, TradeInfo sellTradeInfo):
        m_buyTradeInfo{buyTradeInfo}, m_sellTradeInfo{sellTradeInfo}{}

TradeInfo Trade::getBuyTradeInfo() const{
    return m_buyTradeInfo;
}

TradeInfo Trade::getSellTradeInfo() const{
    return m_sellTradeInfo;
}