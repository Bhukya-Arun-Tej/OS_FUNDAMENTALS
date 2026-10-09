#include <vector>
#include "packet_sequencer.hpp"
#include <iostream>


void drainPendingPackets(std::vector<PendingPacketSlot>& packetBuffer,std::uint32_t& expected)
  {
      while(1){
        const std::size_t capacity = packetBuffer.size();
        const std::size_t bufferIndex = (expected-1) &(capacity-1);
        PendingPacketSlot& slot = packetBuffer[bufferIndex];
        if(!slot.valid || slot.sequenceNumber!=expected){
            return;
        }  
        std::cout<<"Received sequence: "<<expected<<"\n";
        // std::cout<<"Received event type: "<<static_cast<uint16_t> (slot.packet.messageType)<<"\n";
        // AddOrderEvent event;
        // decodeAddOrderPayload(slot.packet.payload,event);
        // std::cout<<"orderId: "<<event.orderId<<" price: "<<event.price<<" quantity: "<<event.quantity<<"\n";
        slot.valid = false;
        expected++;
      }
  }