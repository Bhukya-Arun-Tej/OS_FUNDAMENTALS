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
        Quantity m_remainingQuantity{};
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
        m_quantity(quantity),
        m_remainingQuantity(quantity){}

        Order
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

        Price getPrice() const{
            return m_price;
        }

        OrderId getOrderId() const{
            return m_orderId;
        }

        Quantity getQuantity() const {
            return m_quantity;
        }

        Side getSide() const{
            return m_side;
        }

        Quantity getRemainingQuantity() const{
            return m_remainingQuantity;
        }

        void setQuantity(Quantity newQuantity){
            m_quantity = newQuantity;
        }

        void setRemainingQuantity(Quantity newQuantity){
            m_remainingQuantity=newQuantity;
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

class TradeInfo{
    private:
        OrderId m_orderId;
        Price m_price;
        Quantity m_quantity;
    public:
        TradeInfo() = default;
        TradeInfo(OrderId orderId, Price price, Quantity quantity):
        m_orderId{orderId}, m_price{price}, m_quantity{quantity}{}

        OrderId getOrderId() const{
            return m_orderId;
        }
        Price getPrice() const{
            return m_price;
        }
        Quantity getQuantity() const{
            return m_quantity;
        }
};

class Trade{
    private:
        TradeInfo m_buyTradeInfo{};
        TradeInfo m_sellTradeInfo{};
    public:
        Trade() = default;
        Trade(TradeInfo buyTradeInfo, TradeInfo sellTradeInfo):
        m_buyTradeInfo{buyTradeInfo}, m_sellTradeInfo{sellTradeInfo}{}

        TradeInfo getBuyTradeInfo() const{
            return m_buyTradeInfo;
        }

        TradeInfo getSellTradeInfo() const{
            return m_sellTradeInfo;
        }
};

using BuyMap = std::map<Price,OrderList,std::greater<Price>>;
using SellMap = std::map<Price,OrderList>; 
using Trades = std::vector<Trade>;

class OrderBook{
    private:
        BuyMap m_buyMap;
        SellMap m_sellMap;
        std::unordered_map<OrderId, OrderLocation> m_orderIndexMap;

    public:

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
                std::cout<<"{"<<itr.getOrderId()<<", "<<itr.getRemainingQuantity()<<"}";
            }
            std::cout<<"\n";
        }
    }

    void printSell(){
        const auto & map_t = getSellMap();
        for(auto it:map_t){
            std::cout<<it.first<<": ";
            for(auto itr:it.second){
                std::cout<<"{"<<itr.getOrderId()<<", "<<itr.getRemainingQuantity()<<"}";
            }
            std::cout<<"\n";
        }
    }
    
    bool canMatch(Order &order){
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


    Trades addOrder(OrderId orderid, Side side, Price price, Quantity quantity){
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

    void cancelOrder(OrderId orderid){

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

    void modifyOrder(OrderId orderId, Price newPrice, Quantity newQuantity){
        auto locationPtr = m_orderIndexMap.find(orderId);
        if(locationPtr == m_orderIndexMap.end()){
            return;
        }
        Side side = locationPtr->second.getSide();
        cancelOrder(orderId);
        addOrder(orderId,side,newPrice,newQuantity);

        //invoke matching engine
    }

    
    Trades matchOrder(Order &order){
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

   
};

int main(){
    OrderBook ob;
    ob.addOrder(1,Side::SELL, 100,10);
    ob.addOrder(2,Side::SELL, 130,17);
    ob.addOrder(4,Side::SELL, 90,14);

    ob.printSell();
    auto trades = ob.addOrder(3,Side::BUY, 100, 50);
    for(auto it:trades){
        std::cout<<it.getBuyTradeInfo().getPrice()<<" "<<it.getBuyTradeInfo().getQuantity()<<"\n";
        std::cout<<it.getSellTradeInfo().getPrice()<<" "<<it.getSellTradeInfo().getQuantity()<<"\n\n";
    }

    ob.printBuy();
    ob.printSell();

}


