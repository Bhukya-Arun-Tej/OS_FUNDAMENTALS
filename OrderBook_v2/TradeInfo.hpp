#pragma once
#include "Order.hpp"

class TradeInfo{
private:
    OrderId m_orderId;
    Price m_price;
    Quantity m_quantity;
public:
    TradeInfo() = default;
    TradeInfo(OrderId orderId, Price price, Quantity quantity);

    OrderId getOrderId() const;
    Price getPrice() const;
    Quantity getQuantity() const;
};