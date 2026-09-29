#include "Orderbook.hpp"
#include <iostream>

int main(){
    Orderbook ob;
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
