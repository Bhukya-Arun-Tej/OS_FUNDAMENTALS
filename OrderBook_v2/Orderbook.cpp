#include <cstdint>  
#include <queue>
#include <functional>
#include <iostream>
#include <list>
#include <map>

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
    public:
        Order
        (
            OrderId orderId,
            Side side,
            Price price,
            Quantity quantity
        ): 
        m_orderId(orderId),
        m_side(side),
        m_price(price),
        m_quantity(quantity){}

        Price getPrice() const{
            return m_price;
        }

        OrderId getOrderId() const{
            return m_orderId;
        }
};
using OrderList = std::list<Order>;
using OrderPointer = std::list<Order>::iterator;
class OrderLocation{
    private:
        Price m_price{};
        Side m_side{};
        OrderPointer m_pointer{};
    
    public:
        OrderLocation(Price price, Side side, OrderPointer pointer):
            m_price{price}, m_side{side}, m_pointer{pointer}{}

        OrderPointer getOrderPointer() const{
            return m_pointer;
        }

        Price getPrice() const{
            return m_price;
        }

        Side getSide() const{
            return m_side;
        }
};

using BuyMap = std::map<Price,OrderList,std::greater<Price>>;
using SellMap = std::map<Price,OrderList>; 

class OrderBook{
    private:
        BuyMap m_buyMap;
        SellMap m_sellMap;
        std::unordered_map<OrderId, OrderLocation> m_orderIndexMap;

    public:
    
    void m_addOrder(OrderId orderid, Side side, Price price, Quantity quantity){
        if(m_orderIndexMap.contains(orderid)){
            return; 
        }
        Order order{orderid,side,price,quantity};
        if(side==Side::BUY){
            OrderList &list = m_buyMap[price];
            auto it = list.emplace(
                        list.end(),
                        orderid,
                        side,
                        price,
                        quantity
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
                        quantity
                        );
            m_orderIndexMap.emplace(orderid, OrderLocation{price,side,it});
        }

        //invoke matching engine
    }

    void m_cancelOrder(OrderId orderid){

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

    void m_modifyOrder(OrderId orderId, Price newPrice, Quantity newQuantity){
        auto locationPtr = m_orderIndexMap.find(orderId);
        if(locationPtr == m_orderIndexMap.end()){
            return;
        }
        Side side = locationPtr->second.getSide();
        m_cancelOrder(orderId);
        m_addOrder(orderId,side,newPrice,newQuantity);

        //invoke matching engine
    }

    const BuyMap& getBuyMap() const{
        return m_buyMap;
    }
    const SellMap& getSellMap() const{
        return m_sellMap;
    }

    void printBuy(){
        const auto &map_t = getBuyMap();
        for(auto it:map_t){
            std::cout<<it.first<<": ";
            for(auto itr:it.second){
                std::cout<<itr.getOrderId()<<" ";
            }
            std::cout<<"\n";
        }
    }

    void printSell(){
        const auto & map_t = getSellMap();
        for(auto it:map_t){
            std::cout<<it.first<<": ";
            for(auto itr:it.second){
                std::cout<<itr.getOrderId()<<" ";
            }
            std::cout<<"\n";
        }
    }
};

int main(){
    OrderBook ob;
    ob.m_addOrder(1,Side::BUY, 100,10);
    ob.m_addOrder(2,Side::BUY, 130,17);
    ob.m_addOrder(4,Side::BUY, 90,14);

    ob.printBuy();

    ob.m_cancelOrder(4);
    ob.printBuy();
    ob.m_modifyOrder(2,50,10);
    ob.printBuy();

}


