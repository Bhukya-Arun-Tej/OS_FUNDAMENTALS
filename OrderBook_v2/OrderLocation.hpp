#pragma once
#include "Order.hpp"
#include <list>

using OrderList = std::list<Order>;
using OrderPointer = std::list<Order>::iterator;

class OrderLocation{
private:
    Price m_price{};
    Side m_side{};
    OrderPointer m_pointer{};

public:
    OrderLocation(Price price, Side side, OrderPointer pointer);
    OrderPointer getOrderPointer() const;
    Price getPrice() const;
    Side getSide() const;
};