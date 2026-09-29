#pragma once
#include "Order.hpp"
#include "OrderLocation.hpp"
#include "TradeInfo.hpp"
#include "Trade.hpp"
#include <functional>
#include <map>
#include <unordered_map>
#include <vector>



using BuyMap = std::map<Price,OrderList,std::greater<Price>>;
using SellMap = std::map<Price,OrderList>;
using Trades = std::vector<Trade>;

class Orderbook{
private:
    BuyMap m_buyMap;
    SellMap m_sellMap;
    std::unordered_map<OrderId, OrderLocation> m_orderIndexMap;

public:

    const BuyMap& getBuyMap() const;
    const SellMap& getSellMap() const;
    void printBuy();
    void printSell();
    bool canMatch(Order &order);
    Trades addOrder(OrderId orderid, Side side, Price price, Quantity quantity);
    void cancelOrder(OrderId orderid);
    void modifyOrder(OrderId orderId, Price newPrice, Quantity newQuantity);
    Trades matchOrder(Order &order);
};