#include "TradeInfo.hpp"

TradeInfo::TradeInfo(OrderId orderId, Price price, Quantity quantity):
    m_orderId{orderId}, m_price{price}, m_quantity{quantity}{}

OrderId TradeInfo::getOrderId() const{
    return m_orderId;
}
Price TradeInfo::getPrice() const{
    return m_price;
}
Quantity TradeInfo::getQuantity() const{
    return m_quantity;
}
