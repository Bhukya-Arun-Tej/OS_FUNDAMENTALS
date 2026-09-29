#pragma once
#include <cstdint>

enum class Side{
    BUY,
    SELL,
};

using OrderId = uint64_t;
using TimeStamp = uint64_t;
using Price = uint64_t;
using Quantity = uint64_t;

class Order{
private:
    OrderId m_orderId{};
    Side m_side{};
    Price m_price{};
    Quantity m_quantity{};
    Quantity m_remainingQuantity{};
public:
    Order
            (
                    OrderId orderId,
                    Side side,
                    Price price,
                    Quantity quantity
            );
    Order
            (
                    OrderId orderId,
                    Side side,
                    Price price,
                    Quantity quantity,
                    Quantity remainingQuantity
            );
    Price getPrice() const;
    OrderId getOrderId() const;
    Quantity getQuantity() const;
    Side getSide() const;
    Quantity getRemainingQuantity() const;
    void setQuantity(Quantity newQuantity);
    void setRemainingQuantity(Quantity newQuantity);
};
