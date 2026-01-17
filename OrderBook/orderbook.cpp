#include<iostream>
#include<vector>
#include<map>
#include<unordered_map>
#include<unordered_set>
#include <list>
#include <numeric>


enum Side{
    BUY,
    SELL,
};

enum OrderType{
    Limit,
    Market,
    FillAndKill,
    GoodTillCancel,
};


using Price = std::uint32_t;
using Quantity = std::uint32_t;
using OrderId = std::uint64_t;

class LevelInfo{
    private:
        Price price;
        Quantity quantity;

    public:
        LevelInfo() =default;
        LevelInfo(Price price,Quantity quantity):
        price(price),
        quantity(quantity){}

    Price getPrice() const{
        return price;
    }

    Quantity getQuantity() const{
        return quantity;
    }

};

using LevelInfos = std::vector<LevelInfo>;

class OrderBookLevelInfos{
    private: 
        LevelInfos bids;
        LevelInfos asks;

    public:
        OrderBookLevelInfos(LevelInfos &bids,LevelInfos &asks):
        bids(bids),
        asks(asks){}

        LevelInfos getBids() const{
            return bids;
        }

        LevelInfos getAsks() const{
            return asks;
        }
};



class Order{
    private:
        Side side;
        Price price;
        Quantity initialQuantity;
        Quantity remainingQuantity;
        OrderId orderId;
        OrderType ordertype;

    public:
        Order(Side side,Price price,Quantity quantity,OrderId orderId,OrderType ordertype):
        side(side),
        price(price),
        initialQuantity(quantity),
        remainingQuantity(quantity),
        orderId(orderId),
        ordertype(ordertype){}

        Side getSide() const{
            return side;
        }

        Price getPrice() const{
            return price;
        }
        
        Quantity getinitialQuantity() const{
            return initialQuantity;
        }

        Quantity getRemainingQuantity() const{
            return remainingQuantity;
        }

        Quantity getFilledQuantity() const{
            return initialQuantity - remainingQuantity;
        }

        OrderId getOrderId() const{
            return orderId;
        }

        OrderType getOrderType() const{
            return ordertype;
        }

        void fill (Quantity quantity){
            if(quantity > remainingQuantity){
                throw new std::logic_error(std::format("Order [{}] does not have enough quantity to fill [{}]",getOrderId(),quantity));
            }
            remainingQuantity -= quantity;

        }

};

using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std:: list<OrderPointer>;

class OrderModify{
    private:
        OrderId orderId;
        Side side;
        Price price;
        Quantity quantity;

    public:
        OrderModify (OrderId orderId, Side side, Price price, Quantity quantity):
        orderId(orderId),
        side(side),
        price(price),
        quantity(quantity){}

        OrderId getOrderId() const{
            return orderId;
        }   

        Side getSide() const{
            return side;
        }

        Price getPrice() const{
            return price;
        }

        Quantity getQuantity() const{
            return quantity;
        }

        OrderPointer toOrderPointer(OrderType type) const{
            return std::make_shared<Order>(side,price,quantity,orderId,type);
        }
};

struct TradeInfo{
    OrderId orderId;
    Price price;
    Quantity quantity;
};

    class Trade{
    private:
        TradeInfo bidTrade;
        TradeInfo askTrade;
    public:
        Trade(TradeInfo bidTrade, TradeInfo askTrade)
        :bidTrade(bidTrade),
        askTrade(askTrade){}

    TradeInfo getBidTrade() const {return bidTrade;}
    TradeInfo getAskTrade() const {return askTrade;}

};

using Trades = std::vector<Trade>;


class OrderBook{
    private:
    struct OrderEntry{
        OrderPointer order{nullptr
        };
        OrderPointers::iterator location;
    };

    std::map<Price,OrderPointers> asks;
    std::map<Price, OrderPointers,std::greater<Price>> bids;

    std::unordered_map<OrderId,OrderEntry> orders;

    bool canMatch(Side side, Price price){
        if(side == Side::BUY){
            if(asks.empty()){
                return false;
            }
            const auto &[bestask,_] = *asks.begin();
            return bestask<= price;
        }else{
            if(bids.empty())return false;
            const auto &[bestbid,_] = *bids.begin();
            return bestbid>= price;
        }
    }

    Trades matchOrders(){
        Trades trades;
        trades.reserve(orders.size());

        while(!bids.empty() && !asks.empty()){
            auto &[bestbid,bidlist] = *bids.begin();
            auto &[bestask,asklist] = *asks.begin();

            if(bestbid < bestask){
                break;
            }
            
            while(bidlist.size() and asklist.size()){
                auto &bid = *bidlist.begin();
                auto &ask = *asklist.begin();

                Quantity quantity = std::min(bid->getRemainingQuantity(),ask->getRemainingQuantity()); 
                bid->fill(quantity);
                ask->fill(quantity);

                if(bid->getRemainingQuantity()==0){
                    bidlist.pop_front();
                    orders.erase(bid->getOrderId());
                }
                if(ask->getRemainingQuantity()==0){
                    asklist.pop_front();
                    orders.erase(ask->getOrderId());    
                }

                if(bidlist.empty()){
                    bids.erase(bestbid);
                }
                if(asklist.empty()){
                    asks.erase(bestask);
                }

                trades.push_back(Trade{
                    TradeInfo{bid->getOrderId(),bestbid,quantity},
                    TradeInfo{ask->getOrderId(),bestask,quantity}
                });
            }

        }

        if(!bids.empty()){
            auto &[_,bidlist] = *bids.begin();
            auto &order = bidlist.front();
            if(order->getOrderType()== OrderType::FillAndKill){
                cancelOrder(order->getOrderId());
            }

        }

        if(!asks.empty()){
            auto &[_,asklist] = *asks.begin();
            auto &order = asklist.front();
            if(order->getOrderType() == OrderType::FillAndKill){
                cancelOrder(order->getOrderId());
            }
        }
        return trades;
    }

    public:

    Trades addOrder(OrderPointer order){
        if(orders.contains(order->getOrderId())){
            return {};
        }
        if(order->getOrderType()==OrderType:: FillAndKill and !canMatch(order->getSide(), order->getPrice())){
            return {};
        }

        OrderPointers::iterator location;
        if(order->getSide()==Side::BUY){
            auto &orderlist = bids[order->getPrice()];
            orderlist.push_back(order);
            location = std::prev(orderlist.end());
            
        }else{
            auto &orderlist = asks[order->getPrice()];
            orderlist.push_back(order);
            location = std::prev(orderlist.end());
        }


        orders[order->getOrderId()] = OrderEntry{order, location};

        return matchOrders();
    }

    void cancelOrder(OrderId orderId){
        if(!orders.contains(orderId)){
            return;
        }
        auto &[order, location] = orders[orderId];

        if(order->getSide()==Side::BUY){
            auto price = order->getPrice();
            auto &orderlist = bids[price];
            orderlist.erase(location);
            if(orderlist.empty()){
                bids.erase(price);
            }
        }else{
            auto price = order->getPrice();
            auto &orderlist = asks[price];
            orderlist.erase(location);
            if(orderlist.empty()){
                asks.erase(price);
            }
        }
        orders.erase(orderId);
    }
    
    Trades ModifyOrder(OrderModify order){
        if(!orders.contains(order.getOrderId())){
            return {};
        }

        const auto&[existingOrder, _ ] = orders[order.getOrderId()];

        cancelOrder(order.getOrderId());
        auto newOrder = order.toOrderPointer(existingOrder->getOrderType());
        return addOrder(newOrder);
    } 

    std::size_t Size(){return orders.size();}

    OrderBookLevelInfos getOrderInfos() const{
        LevelInfos bidInfos, askInfos;
        bidInfos.reserve(orders.size());
        askInfos.reserve(orders.size());

        auto CreateLevelInfos = [](Price price, const OrderPointers& orderlist){
            return LevelInfo{price,
                std::accumulate(
                    orderlist.begin(),
                    orderlist.end(),
                    (Quantity) 0,
                    [](Quantity runningSum, const OrderPointer& order){
                        return runningSum+ order->getRemainingQuantity();
                    }
                )
            };
        };

        for (const auto& [price, orderlist]: bids){
            bidInfos.push_back(CreateLevelInfos(price, orderlist));
        }

        for(const auto& [price, orderlist]: asks){
            askInfos.push_back(CreateLevelInfos(price, orderlist));
        }
        return OrderBookLevelInfos{bidInfos,askInfos};
    }

};


int main(){
    OrderBook orderBook;
    const OrderId orderId = 1;
    orderBook.addOrder(std:: make_shared<Order>( Side::BUY, 100, 10, orderId, OrderType::Limit));
    std:: cout<< orderBook.Size()<<std::endl;
    orderBook.cancelOrder(orderId);
    std:: cout<< orderBook.Size()<<std::endl;
    return 0;
}
