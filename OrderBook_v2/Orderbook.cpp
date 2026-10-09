#include "Orderbook.hpp"

#include <queue>
#include <functional>
#include <iostream>
#include <list>
#include <map>


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
        if(price>=m_sellMap.begin()->first)return true;
    }
    else{
        //match with buy orders
        if(m_buyMap.empty())return false;
        if(price<=m_buyMap.begin()->first)return true;
    }
    return false;
}

bool Orderbook::addRestingOrder(OrderId orderid, Side side, Price price, Quantity quantity){
    return addRestingOrder(orderid,side,price,quantity,quantity);
}

bool Orderbook::addRestingOrder(OrderId orderid, Side side, Price price, Quantity quantity, Quantity remainingQuantity){
    if(m_orderIndexMap.contains(orderid)){
        return false;
    }

    if(side==Side::BUY){
        OrderList &list = m_buyMap[price];
        auto it = list.emplace(
                list.end(),
                orderid,
                side,
                price,
                quantity,
                remainingQuantity
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
                remainingQuantity
        );
        m_orderIndexMap.emplace(orderid, OrderLocation{price,side,it});
    }
    return true;
}

Trades Orderbook::addOrder(OrderId orderid, Side side, Price price, Quantity quantity){
    if(m_orderIndexMap.contains(orderid)){
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

    addRestingOrder(orderid,side,price,quantity,order.getRemainingQuantity());
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

    Trades trades;

    if(side==Side::BUY){
        // match with sell orders
        Quantity buyQuantity = order.getRemainingQuantity();
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

        std::vector<OrderId> removeOrders;
        Quantity sellQuantity = order.getRemainingQuantity();
        for(auto mapItr = m_buyMap.begin(); mapItr!=m_buyMap.end();){
            if((mapItr->first) < price or (sellQuantity==0)){
                // return trades;
                break;
            }

            for(auto it=mapItr->second.begin();it!=mapItr->second.end();){
                if(it->getRemainingQuantity()<=sellQuantity){
                    sellQuantity -= it->getRemainingQuantity();
                    trades.emplace_back(
                            Trade{
                                    TradeInfo{it->getOrderId(),it->getPrice(), it->getRemainingQuantity()},
                                    TradeInfo{order.getOrderId(),it->getPrice(), it->getRemainingQuantity()},
                            }
                    );
                    removeOrders.push_back(it->getOrderId());
                    // mapItr->second.erase(it);
                    // cancelOrder(it->getOrderId());
                }
                else{
                    trades.emplace_back(
                            Trade{
                                    TradeInfo{it->getOrderId(),it->getPrice(), sellQuantity},
                                    TradeInfo{order.getOrderId(),it->getPrice(), sellQuantity},
                            }
                    );
                    it->setRemainingQuantity(it->getRemainingQuantity()-sellQuantity);
                    sellQuantity=0;
                    // break;
                }
                if(sellQuantity==0)break;
                it++;
            }
            mapItr++;
        }

        // if(buyQuantity!=0){
        // order.setQuantity(buyQuantity);
        order.setRemainingQuantity(sellQuantity);
        // }
        for(const auto &it: removeOrders){
            cancelOrder(it);
        }
    }
    return trades;
}


