#include "Orderbook.hpp"

#include <queue>
#include <functional>
#include <iostream>
#include <list>
#include <map>

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

//TradeInfo::TradeInfo() = default;
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



Trade::Trade(TradeInfo buyTradeInfo, TradeInfo sellTradeInfo):
        m_buyTradeInfo{buyTradeInfo}, m_sellTradeInfo{sellTradeInfo}{}

TradeInfo Trade::getBuyTradeInfo() const{
    return m_buyTradeInfo;
}

TradeInfo Trade::getSellTradeInfo() const{
    return m_sellTradeInfo;
}

const BuyMap& Orderbook::getBuyMap() const{
    return m_buyMap;
}
const SellMap& Orderbook::getSellMap() const{
    return m_sellMap;
}

void Orderbook::printBuy(){
    const auto &map_t = getBuyMap();
    for(auto it:map_t){
        std::cout<<it.first<<": ";
        for(auto itr:it.second){
            std::cout<<"{"<<itr.getOrderId()<<", "<<itr.getRemainingQuantity()<<"}";
        }
        std::cout<<"\n";
    }
}

void Orderbook::printSell(){
    const auto & map_t = getSellMap();
    for(auto it:map_t){
        std::cout<<it.first<<": ";
        for(auto itr:it.second){
            std::cout<<"{"<<itr.getOrderId()<<", "<<itr.getRemainingQuantity()<<"}";
        }
        std::cout<<"\n";
    }
}

bool Orderbook::canMatch(Order &order){
    const Price &price = order.getPrice();
    const Side &side = order.getSide();

    if(side==Side::BUY){
        // match with sell orders
        if(m_sellMap.empty())return false;
        if(price>m_sellMap.begin()->first)return true;
    }
    else{
        //match with buy orders
        if(m_buyMap.empty())return false;
        if(price<=m_buyMap.begin()->first)return true;
    }
    return false;
}


Trades Orderbook::addOrder(OrderId orderid, Side side, Price price, Quantity quantity){
    if(m_orderIndexMap.find(orderid) != m_orderIndexMap.end()){
        return {};
    }
    Trades trades{};
    Order order{orderid,side,price,quantity};
    if(canMatch(order)){
        trades = matchOrder(order);
    }
    if(order.getRemainingQuantity()==0){
        return trades;
    }

    if(side==Side::BUY){
        OrderList &list = m_buyMap[price];
        auto it = list.emplace(
                list.end(),
                orderid,
                side,
                price,
                quantity,
                order.getRemainingQuantity()
        );
        m_orderIndexMap.emplace(orderid, OrderLocation{price,side,it});
    }
    else{
        OrderList &list = m_sellMap[price];
        auto it = list.emplace(
                list.end(),
                orderid,
                side,
                price,
                quantity,
                order.getRemainingQuantity()
        );
        m_orderIndexMap.emplace(orderid, OrderLocation{price,side,it});
    }

    return trades;
}

void Orderbook::cancelOrder(OrderId orderid){

    auto locationPtr = m_orderIndexMap.find(orderid);
    if(locationPtr==m_orderIndexMap.end()){
        return;
    }

    const OrderLocation &location = locationPtr->second;
    // OrderList *list;
    Price price = location.getPrice();
    OrderPointer pointer = location.getOrderPointer();
    if(location.getSide() == Side::BUY){
        auto level = m_buyMap.find(price);
        auto &list = level->second;
        list.erase(pointer);
        if(list.empty()){
            m_buyMap.erase(level);
        }
    }
    else{
        auto level = m_sellMap.find(price);
        auto &list = level->second;

        list.erase(pointer);
        if(list.empty()){
            m_sellMap.erase(level);
        }
    }
    m_orderIndexMap.erase(locationPtr);

    // invoke matching engine
}

void Orderbook::modifyOrder(OrderId orderId, Price newPrice, Quantity newQuantity){
    auto locationPtr = m_orderIndexMap.find(orderId);
    if(locationPtr == m_orderIndexMap.end()){
        return;
    }
    Side side = locationPtr->second.getSide();
    cancelOrder(orderId);
    addOrder(orderId,side,newPrice,newQuantity);

    //invoke matching engine
}


Trades Orderbook::matchOrder(Order &order){
    const Price &price = order.getPrice();
    const Side &side = order.getSide();
    Quantity buyQuantity = order.getRemainingQuantity();

    Trades trades;

    if(side==Side::BUY){
        // match with sell orders
        std::vector<OrderId> removeOrders;
        for(auto mapItr = m_sellMap.begin(); mapItr!=m_sellMap.end();){
            if((mapItr->first) >price or (buyQuantity==0)){
                // return trades;
                break;
            }

            for(auto it=mapItr->second.begin();it!=mapItr->second.end();){
                if(it->getRemainingQuantity()<=buyQuantity){
                    buyQuantity -= it->getRemainingQuantity();
                    trades.emplace_back(
                            Trade{
                                    TradeInfo{order.getOrderId(),it->getPrice(), it->getRemainingQuantity()},
                                    TradeInfo{it->getOrderId(),it->getPrice(), it->getRemainingQuantity()},
                            }
                    );
                    removeOrders.push_back(it->getOrderId());
                    // mapItr->second.erase(it);
                    // cancelOrder(it->getOrderId());
                }
                else{
                    trades.emplace_back(
                            Trade{
                                    TradeInfo{order.getOrderId(),it->getPrice(), buyQuantity},
                                    TradeInfo{it->getOrderId(),it->getPrice(), buyQuantity},
                            }
                    );
                    it->setRemainingQuantity(it->getRemainingQuantity()-buyQuantity);
                    buyQuantity=0;
                    // break;
                }
                if(buyQuantity==0)break;
                it++;
            }
            mapItr++;
        }

        // if(buyQuantity!=0){
        // order.setQuantity(buyQuantity);
        order.setRemainingQuantity(buyQuantity);
        // }
        for(const auto &it: removeOrders){
            cancelOrder(it);
        }


    }
    else{
        //match with buy orders

    }
    return trades;
}


