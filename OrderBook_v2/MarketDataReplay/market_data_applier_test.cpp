#include "market_data_applier.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

class MarketDataApplierTest {
public:
    int runAllTests() {
        bool allPassed = true;

        if (addOrderPacketAddsRestingOrder()) {
            std::cout << "PASS: addOrderPacketAddsRestingOrder\n";
        } else {
            std::cout << "FAIL: addOrderPacketAddsRestingOrder\n";
            allPassed = false;
        }

        if (cancelOrderPacketRemovesRestingOrder()) {
            std::cout << "PASS: cancelOrderPacketRemovesRestingOrder\n";
        } else {
            std::cout << "FAIL: cancelOrderPacketRemovesRestingOrder\n";
            allPassed = false;
        }

        if (malformedAddOrderPacketDoesNotMutateBook()) {
            std::cout << "PASS: malformedAddOrderPacketDoesNotMutateBook\n";
        } else {
            std::cout << "FAIL: malformedAddOrderPacketDoesNotMutateBook\n";
            allPassed = false;
        }

        if (testPacketIsRejected()) {
            std::cout << "PASS: testPacketIsRejected\n";
        } else {
            std::cout << "FAIL: testPacketIsRejected\n";
            allPassed = false;
        }

        return allPassed ? 0 : 1;
    }

private:
    bool addOrderPacketAddsRestingOrder() {
        Orderbook book;
        MarketDataApplier applier(book);

        const AddOrderEvent event{1, FeedSide::Buy, 100, 10};
        const FeedPacket packet{
            1,
            MessageType::AddOrder,
            encodeAddOrderPayload(event)
        };

        assert(applier.processPacket(packet) && "valid AddOrder packet should apply");
        assert(book.getBuyMap().size() == 1 && "one buy price level should exist");
        assert(book.getBuyMap().begin()->first == 100 && "buy price should be 100");
        assert(book.getBuyMap().begin()->second.front().getOrderId() == 1 && "order ID should be 1");
        assert(book.getBuyMap().begin()->second.front().getRemainingQuantity() == 10 && "quantity should be 10");
        return true;
    }

    bool cancelOrderPacketRemovesRestingOrder() {
        Orderbook book;
        MarketDataApplier applier(book);

        assert(book.addRestingOrder(1, Side::BUY, 100, 10));

        const CancelOrderEvent event{1};
        const FeedPacket packet{
            2,
            MessageType::CancelOrder,
            encodeCancelOrderPayload(event)
        };

        assert(applier.processPacket(packet) && "valid CancelOrder packet should apply");
        assert(book.getBuyMap().empty() && "cancel should remove the resting order");
        return true;
    }

    bool malformedAddOrderPacketDoesNotMutateBook() {
        Orderbook book;
        MarketDataApplier applier(book);

        const FeedPacket packet{
            1,
            MessageType::AddOrder,
            std::vector<std::uint8_t>{}
        };

        assert(!applier.processPacket(packet) && "malformed AddOrder packet should be rejected");
        assert(book.getBuyMap().empty() && "rejected packet must not add a buy order");
        assert(book.getSellMap().empty() && "rejected packet must not add a sell order");
        return true;
    }

    bool testPacketIsRejected() {
        Orderbook book;
        MarketDataApplier applier(book);

        const FeedPacket packet{
            1,
            MessageType::Test,
            std::vector<std::uint8_t>{}
        };

        assert(!applier.processPacket(packet) && "Test packet should not update the local book");
        return true;
    }
};

int main() {
    MarketDataApplierTest test;
    return test.runAllTests();
}
