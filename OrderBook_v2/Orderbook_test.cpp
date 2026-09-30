#include "Orderbook.hpp"
#include <iostream>
#include <string>
#include <cassert>

class OrderbookTest{
    public:
        int runAllTests(){
            bool allPassed = true;
            if(buyAtBestAskExecutes()){
                std::cout<<"PASS: buyAtBestAskExecutes\n";
            }
            else{
                std::cout<<"FAIL: buyAtBestAskExecutes\n";
                allPassed = false;
            }


            if (crossingSellExecutesAgainstBestBid()) {
                std::cout << "PASS: crossingSellExecutesAgainstBestBid\n";
            } else {
                std::cout << "FAIL: crossingSellExecutesAgainstBestBid\n";
                allPassed = false;
            }
            return allPassed?0 : 1;
        }
    
    private:
        bool buyAtBestAskExecutes(){
            Orderbook book;
            book.addOrder(1,Side::SELL,100,10);

            Trades trades = book.addOrder(2, Side::BUY, 100,10);
            assert(trades.size()==1 && "one trade should be created");
            assert(trades[0].getBuyTradeInfo().getOrderId()==2 && "buy order ID should be 2");
            assert(trades[0].getSellTradeInfo().getOrderId()==1 && "sell order ID should be 1");
            assert(trades[0].getBuyTradeInfo().getPrice()==100 && trades[0].getSellTradeInfo().getPrice()==100 && "trade price should be 100");
            assert(trades[0].getBuyTradeInfo().getQuantity()==10 &&  trades[0].getSellTradeInfo().getQuantity()==10 && "trade quantity should be 10");
            assert(book.getBuyMap().empty() && "fully filled buy should be empty");
            assert(book.getSellMap().empty() && "fully filled sell should be empty");

            return true;
        }

        bool crossingSellExecutesAgainstBestBid() {
            Orderbook book;
            book.addOrder(10, Side::BUY, 100, 7);
            Trades trades = book.addOrder(20,Side::SELL, 100,7);
            assert(trades.size()==1 && "one trade should be created");
            assert(trades[0].getBuyTradeInfo().getOrderId()==10 && "buy order ID should be 10");
            assert(trades[0].getSellTradeInfo().getOrderId()==20 && "sell order ID should be 20");
            return true;
        }
    
};

int main(){
    OrderbookTest test;
    return test.runAllTests();
}