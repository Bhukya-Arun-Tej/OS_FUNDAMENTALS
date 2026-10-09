#include "Order.hpp"

Order::Order
    (
        OrderId orderId,
        Side side,
        Price price,
        Quantity quantity
    ):
    m_orderId(orderId),
    m_side(side),
    m_price(price),
    m_quantity(quantity),
    m_remainingQuantity(quantity){}

Order:: Order
    (
        OrderId orderId,
        Side side,
        Price price,
        Quantity quantity,
        Quantity remainingQuantity
    ):
    m_orderId(orderId),
    m_side(side),
    m_price(price),
    m_quantity(quantity),
    m_remainingQuantity(remainingQuantity){}

Price Order::getPrice() const{
    return m_price;
}

OrderId Order::getOrderId() const{
    return m_orderId;
}

Quantity Order::getQuantity() const {
    return m_quantity;
}

Side Order::getSide() const{
    return m_side;
}

Quantity Order::getRemainingQuantity() const{
    return m_remainingQuantity;
}

void Order::setQuantity(Quantity newQuantity){
m_quantity = newQuantity;
}

void Order::setRemainingQuantity(Quantity newQuantity){
m_remainingQuantity=newQuantity;
}