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


            if(nonCrossingBuyRests()){
                std::cout<<"PASS: nonCrossingBuyRests\n";
            }
            else{
                std::cout<<"FAIL: nonCrossingBuyRests\n";
                allPassed = false;
            }

            if(partialFillLeavesIncomingRemainderResting()){
                std::cout<<"PASS: partialFillLeavesIncomingRemainderResting\n";
            }
            else{
                std::cout<<"FAIL: partialFillLeavesIncomingRemainderResting\n";
                allPassed = false;
            }

            if(partialFillLeavesRestingRemainder()){
                std::cout<<"PASS: partialFillLeavesRestingRemainder\n";
            }
            else{
                std::cout<<"FAIL: partialFillLeavesRestingRemainder\n";
                allPassed = false;
            }

            if(buyAboveBestAskExecutesAtRestingPrice()){
                std::cout<<"PASS: buyAboveBestAskExecutesAtRestingPrice\n";
            }
            else{
                std::cout<<"FAIL: buyAboveBestAskExecutesAtRestingPrice\n";
                allPassed = false;
            }

            if(bestPriceHasPriority()){
                std::cout<<"PASS: bestPriceHasPriority\n";
            }
            else{
                std::cout<<"FAIL: bestPriceHasPriority\n";
                allPassed = false;
            }

            if(firstOrderAtPriceHasFifoPriority()){
                std::cout<<"PASS: firstOrderAtPriceHasFifoPriority\n";
            }
            else{
                std::cout<<"FAIL: firstOrderAtPriceHasFifoPriority\n";
                allPassed = false;
            }

            if(cancelRemovesOrderAndEmptyPriceLevel()){
                std::cout<<"PASS: cancelRemovesOrderAndEmptyPriceLevel\n";
            }
            else{
                std::cout<<"FAIL: cancelRemovesOrderAndEmptyPriceLevel\n";
                allPassed = false;
            }

            if(modifyReplacesOrderAtNewPriceAndQuantity()){
                std::cout<<"PASS: modifyReplacesOrderAtNewPriceAndQuantity\n";
            }
            else{
                std::cout<<"FAIL: modifyReplacesOrderAtNewPriceAndQuantity\n";
                allPassed = false;
            }

            if(addRestingOrderDoesNotMatch()){
                std::cout<<"PASS: addRestingOrderDoesNotMatch\n";
            }
            else{
                std::cout<<"FAIL: addRestingOrderDoesNotMatch\n";
                allPassed = false;
            }

            if(addRestingOrderDuplicateFails()){
                std::cout<<"PASS: addRestingOrderDuplicateFails\n";
            }
            else{
                std::cout<<"FAIL: addRestingOrderDuplicateFails\n";
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

        bool nonCrossingBuyRests() {
            Orderbook book;
            book.addOrder(1, Side::SELL, 101, 10);
            Trades trades = book.addOrder(2, Side::BUY, 100, 10);
            assert(trades.empty() && "trades generated are 0");
            assert(book.getBuyMap().size() == 1 && "buy order must have only 1 price level");
            assert(book.getSellMap().size() == 1 && "sell order must have only 1 price level");
            return true;
        }

        bool partialFillLeavesIncomingRemainderResting(){
            Orderbook book;
            book.addOrder(1,Side::SELL,100,5);
            Trades trades = book.addOrder(2,Side::BUY,100,10);
            assert(trades.size()==1 && "one trade should be created");
            assert(trades[0].getBuyTradeInfo().getQuantity()==5 && "buy trade quantity should be 5");
            assert(trades[0].getSellTradeInfo().getQuantity()==5 && "sell trade quantity should be 5");
            assert(book.getSellMap().empty() && "fully filled sell should be removed");
            assert(book.getBuyMap().size()==1 && "buy remainder should rest at one price level");
            assert(book.getBuyMap().begin()->second.front().getOrderId()==2 && "resting buy order ID should be 2");
            assert(book.getBuyMap().begin()->second.front().getQuantity() == 10 && "resting buy should preserve its original quantity");
            assert(book.getBuyMap().begin()->second.front().getRemainingQuantity()==5 && "buy remainder should be 5");
            return true;
        }

        bool partialFillLeavesRestingRemainder(){
            Orderbook book;
            book.addOrder(1,Side::SELL,100,10);
            Trades trades = book.addOrder(2,Side::BUY,100,5);
            assert(trades.size()==1 && "one trade should be created");
            assert(trades[0].getBuyTradeInfo().getQuantity()==5 && "buy trade quantity should be 5");
            assert(trades[0].getSellTradeInfo().getQuantity()==5 && "sell trade quantity should be 5");
            assert(book.getBuyMap().empty() && "fully filled buy should be removed");
            assert(book.getSellMap().size()==1 && "sell remainder should remain at one price level");
            assert(book.getSellMap().begin()->second.front().getOrderId()==1 && "resting sell order ID should be 1");
            assert(book.getSellMap().begin()->second.front().getRemainingQuantity()==5 && "sell remainder should be 5");
            return true;
        }

        bool buyAboveBestAskExecutesAtRestingPrice(){
            Orderbook book;
            book.addOrder(1,Side::SELL,100,10);
            Trades trades = book.addOrder(2,Side::BUY,101,10);
            assert(trades.size()==1 && "one trade should be created");
            assert(trades[0].getBuyTradeInfo().getPrice()==100 && "buy should execute at resting sell price");
            assert(trades[0].getSellTradeInfo().getPrice()==100 && "sell should execute at resting sell price");
            assert(book.getBuyMap().empty() && "fully filled buy should be removed");
            assert(book.getSellMap().empty() && "fully filled sell should be removed");
            return true;
        }

        bool bestPriceHasPriority(){
            Orderbook book;
            book.addOrder(1,Side::SELL,101,10);
            book.addOrder(2,Side::SELL,100,10);
            Trades trades = book.addOrder(3,Side::BUY,101,15);
            assert(trades.size()==2 && "two trades should be created");
            assert(trades[0].getSellTradeInfo().getOrderId()==2 && "best ask should trade first");
            assert(trades[0].getSellTradeInfo().getPrice()==100 && "first trade should be at best ask");
            assert(trades[0].getSellTradeInfo().getQuantity()==10 && "first trade quantity should be 10");
            assert(trades[1].getSellTradeInfo().getOrderId()==1 && "worse ask should trade second");
            assert(trades[1].getSellTradeInfo().getPrice()==101 && "second trade should be at next ask");
            assert(trades[1].getSellTradeInfo().getQuantity()==5 && "second trade quantity should be 5");
            assert(book.getSellMap().size()==1 && "only one sell price level should remain");
            assert(book.getSellMap().begin()->second.front().getOrderId()==1 && "remaining sell should be order 1");
            assert(book.getSellMap().begin()->second.front().getRemainingQuantity()==5 && "remaining sell quantity should be 5");
            return true;
        }

        bool firstOrderAtPriceHasFifoPriority(){
            Orderbook book;
            book.addOrder(1,Side::SELL,100,4);
            book.addOrder(2,Side::SELL,100,6);
            Trades trades = book.addOrder(3,Side::BUY,100,5);
            assert(trades.size()==2 && "two trades should be created");
            assert(trades[0].getSellTradeInfo().getOrderId()==1 && "first resting sell should trade first");
            assert(trades[0].getSellTradeInfo().getQuantity()==4 && "first trade quantity should be 4");
            assert(trades[1].getSellTradeInfo().getOrderId()==2 && "second resting sell should trade second");
            assert(trades[1].getSellTradeInfo().getQuantity()==1 && "second trade quantity should be 1");
            assert(book.getSellMap().begin()->second.front().getOrderId()==2 && "second sell should remain resting");
            assert(book.getSellMap().begin()->second.front().getRemainingQuantity()==5 && "second sell remainder should be 5");
            return true;
        }

        bool cancelRemovesOrderAndEmptyPriceLevel(){
            Orderbook book;
            book.addOrder(1,Side::BUY,100,10);
            book.cancelOrder(1);
            assert(book.getBuyMap().empty() && "cancel should remove the order and its empty price level");
            book.cancelOrder(1);
            assert(book.getBuyMap().empty() && "cancelling a missing order should do nothing");
            return true;
        }

        bool modifyReplacesOrderAtNewPriceAndQuantity(){
            Orderbook book;
            book.addOrder(1,Side::BUY,100,10);
            book.modifyOrder(1,101,4);
            assert(book.getBuyMap().size()==1 && "modified order should have one price level");
            assert(book.getBuyMap().begin()->first==101 && "modified order should rest at new price");
            assert(book.getBuyMap().begin()->second.front().getOrderId()==1 && "modified order ID should remain 1");
            assert(book.getBuyMap().begin()->second.front().getQuantity()==4 && "modified order quantity should be 4");
            assert(book.getBuyMap().begin()->second.front().getRemainingQuantity()==4 && "modified order remaining quantity should be 4");
            return true;
        }

        bool addRestingOrderDoesNotMatch(){
            Orderbook book;
            book.addOrder(1,Side::SELL,100,10);
            const bool added = book.addRestingOrder(2,Side::BUY,191,5);
            assert(added && "resting order should succeed");
            assert(book.getBuyMap().size()==1 && "buy map should have only one entry") ;
            assert(book.getSellMap().size()==1 && "sell map should have only one entry") ;
            return true;
        }

        bool addRestingOrderDuplicateFails(){
            Orderbook book;
            book.addOrder(1,Side::SELL,100,10);
            const bool added = book.addRestingOrder(1,Side::BUY,191,5);
            assert(!added && "resting order should fail");
            return true;
        }

};

int main(){
    OrderbookTest test;
    return test.runAllTests();
}
