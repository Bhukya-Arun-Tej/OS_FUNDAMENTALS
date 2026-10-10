#include "market_data_applier.hpp"


MarketDataApplier:: MarketDataApplier(Orderbook& book): m_book(book){}

bool MarketDataApplier::processPacket(const FeedPacket& packet){

    switch (packet.messageType){
        case MessageType::AddOrder :{
            AddOrderEvent event{};
            if(!decodeAddOrderPayload(packet.payload,event)){
                return false;
            }
            
            Side side;
            if(event.side== FeedSide::Buy){
                side = Side::BUY;
            }
            else{
                side = Side::SELL;
            }
            return m_book.addRestingOrder(event.orderId,side,event.price,event.quantity);
        }
        case MessageType::CancelOrder :{
            CancelOrderEvent event{};
            if(!decodeCancelOrderPayload(packet.payload, event)){
                return false;
            }

            m_book.cancelOrder(event.orderId);
            return true;
        }
        default: {
            return false;
        } 
        return true;
    }
}

