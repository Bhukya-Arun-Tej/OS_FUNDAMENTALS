#include "OrderLocation.hpp"


OrderLocation::OrderLocation(Price price, Side side, OrderPointer pointer):
m_price{price}, m_side{side}, m_pointer{pointer}{}

OrderPointer OrderLocation::getOrderPointer() const{
    return m_pointer;
}

Price OrderLocation::getPrice() const{
    return m_price;
}

Side OrderLocation::getSide() const{
    return m_side;
}
