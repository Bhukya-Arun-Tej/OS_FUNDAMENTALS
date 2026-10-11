#include "market_data_receiver.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>

class MarketDataReceiverTest {
public:
    int runAllTests() {
        bool allPassed = true;

        if (emptyReceiverDoesNotAdvanceSequence()) {
            std::cout << "PASS: emptyReceiverDoesNotAdvanceSequence\n";
        } else {
            std::cout << "FAIL: emptyReceiverDoesNotAdvanceSequence\n";
            allPassed = false;
        }

        if (appliesExpectedAddOrder()) {
            std::cout << "PASS: appliesExpectedAddOrder\n";
        } else {
            std::cout << "FAIL: appliesExpectedAddOrder\n";
            allPassed = false;
        }

        if (drainsPreviouslyBufferedPacket()) {
            std::cout << "PASS: drainsPreviouslyBufferedPacket\n";
        } else {
            std::cout << "FAIL: drainsPreviouslyBufferedPacket\n";
            allPassed = false;
        }

        if (rejectsDuplicatePacket()) {
            std::cout << "PASS: rejectsDuplicatePacket\n";
        } else {
            std::cout << "FAIL: rejectsDuplicatePacket\n";
            allPassed = false;
        }

        if (rejectsPacketOutsideWindow()) {
            std::cout << "PASS: rejectsPacketOutsideWindow\n";
        } else {
            std::cout << "FAIL: rejectsPacketOutsideWindow\n";
            allPassed = false;
        }

        if (appliesAddThenCancelInSequence()) {
            std::cout << "PASS: appliesAddThenCancelInSequence\n";
        } else {
            std::cout << "FAIL: appliesAddThenCancelInSequence\n";
            allPassed = false;
        }

        if (doesNotAdvanceForInvalidEvent()) {
            std::cout << "PASS: doesNotAdvanceForInvalidEvent\n";
        } else {
            std::cout << "FAIL: doesNotAdvanceForInvalidEvent\n";
            allPassed = false;
        }

        if (recoversGapAndDrainsBufferedPackets()) {
            std::cout << "PASS: recoversGapAndDrainsBufferedPackets\n";
        } else {
            std::cout << "FAIL: recoversGapAndDrainsBufferedPackets\n";
            allPassed = false;
        }

        return allPassed ? 0 : 1;
    }

private:
    FeedPacket makeAddOrderPacket(std::uint32_t sequenceNumber,OrderId orderId,FeedSide side = FeedSide::Buy,Price price = 100,Quantity quantity = 10) {
        const AddOrderEvent event{orderId, side, price, quantity};
        return FeedPacket{sequenceNumber,MessageType::AddOrder,encodeAddOrderPayload(event)};
    }

    FeedPacket makeCancelOrderPacket(std::uint32_t sequenceNumber, OrderId orderId) {
        const CancelOrderEvent event{orderId};
        return FeedPacket{sequenceNumber,MessageType::CancelOrder,encodeCancelOrderPayload(event)};
    }

    bool emptyReceiverDoesNotAdvanceSequence() {
        Orderbook book;
        MarketDataApplier applier(book);
        MarketDataReceiver receiver(applier);
        assert(receiver.drainPackets());
        assert(receiver.expectedSequence() == 1 && "empty receiver must expect sequence 1");
        return true;
    }

    bool appliesExpectedAddOrder() {
        Orderbook book;
        MarketDataApplier applier(book);
        MarketDataReceiver receiver(applier);
        const FeedPacket packet = makeAddOrderPacket(1, 10, FeedSide::Buy, 101, 20);
        assert(receiver.bufferPacket(packet));
        assert(receiver.drainPackets());
        assert(receiver.expectedSequence() == 2 && "sequence 1 must advance expected to 2");
        assert(book.getBuyMap().size() == 1 && "AddOrder must add one buy price level");
        assert(book.getBuyMap().begin()->first == 101 && "buy price must match event price");
        assert(book.getBuyMap().begin()->second.front().getOrderId() == 10 && "order ID must match event");
        return true;
    }

    bool drainsPreviouslyBufferedPacket() {
        Orderbook book;
        MarketDataApplier applier(book);
        MarketDataReceiver receiver(applier);
        assert(receiver.bufferPacket(makeAddOrderPacket(3, 30, FeedSide::Buy, 103)));
        assert(receiver.drainPackets());
        assert(receiver.expectedSequence() == 1 && "future packet must remain buffered");
        assert(receiver.bufferPacket(makeAddOrderPacket(1, 10, FeedSide::Buy, 101)));
        assert(receiver.bufferPacket(makeAddOrderPacket(2, 20, FeedSide::Buy, 102)));
        assert(receiver.drainPackets());
        assert(receiver.expectedSequence() == 4 && "draining 1 and 2 must also drain buffered 3");
        assert(book.getBuyMap().size() == 3 && "all three AddOrder events must apply");
        return true;
    }

    bool rejectsDuplicatePacket() {
        Orderbook book;
        MarketDataApplier applier(book);
        MarketDataReceiver receiver(applier);
        const FeedPacket packet = makeAddOrderPacket(1, 10, FeedSide::Buy, 101);
        assert(receiver.bufferPacket(packet));
        assert(receiver.drainPackets());
        assert(!receiver.bufferPacket(packet) && "already applied sequence must be rejected as duplicate");
        assert(receiver.expectedSequence() == 2 && "duplicate must not change expected sequence");
        assert(book.getBuyMap().size() == 1 && "duplicate must not change book state");
        return true;
    }

    bool rejectsPacketOutsideWindow() {
        Orderbook book;
        MarketDataApplier applier(book);
        MarketDataReceiver receiver(applier);
        const FeedPacket packet = makeAddOrderPacket(BUFFERSIZE + 1, 10);
        assert(!receiver.bufferPacket(packet) && "packet at pending-buffer limit must be rejected");
        assert(receiver.expectedSequence() == 1 && "out-of-window packet must not advance expected");
        assert(book.getBuyMap().empty() && "out-of-window packet must not update book");
        return true;
    }

    bool appliesAddThenCancelInSequence() {
        Orderbook book;
        MarketDataApplier applier(book);
        MarketDataReceiver receiver(applier);
        assert(receiver.bufferPacket(makeAddOrderPacket(1, 10, FeedSide::Buy, 101)));
        assert(receiver.bufferPacket(makeCancelOrderPacket(2, 10)));
        assert(receiver.drainPackets());
        assert(receiver.expectedSequence() == 3 && "add and cancel must both advance expected");
        assert(book.getBuyMap().empty() && "CancelOrder must remove the added order");
        return true;
    }

    bool doesNotAdvanceForInvalidEvent() {
        Orderbook book;
        MarketDataApplier applier(book);
        MarketDataReceiver receiver(applier);
        const FeedPacket packet{1, MessageType::Test, {}};
        assert(receiver.bufferPacket(packet));
        assert(!receiver.drainPackets() && "invalid event must fail application");
        assert(receiver.expectedSequence() == 1 && "failed event must not advance expected");
        assert(book.getBuyMap().empty() && "failed event must not update book");
        assert(book.getSellMap().empty() && "failed event must not update book");
        return true;
    }

    bool recoversGapAndDrainsBufferedPackets() {
        Orderbook book;
        MarketDataApplier applier(book);
        MarketDataReceiver receiver(applier);

        for (std::uint32_t sequenceNumber = 1; sequenceNumber <= 4; ++sequenceNumber) {
            assert(receiver.bufferPacket(makeAddOrderPacket(sequenceNumber,sequenceNumber,FeedSide::Buy,100 + sequenceNumber,sequenceNumber)));
            assert(receiver.drainPackets());
        }
        assert(receiver.expectedSequence() == 5 && "sequences 1 through 4 must apply");
        assert(receiver.bufferPacket(makeAddOrderPacket(7, 7, FeedSide::Buy, 107, 7)));
        assert(receiver.drainPackets());
        assert(receiver.expectedSequence() == 5 && "sequence 7 must remain buffered while 5 is missing");
        assert(receiver.bufferPacket(makeAddOrderPacket(5, 5, FeedSide::Buy, 105, 5)));
        assert(receiver.bufferPacket(makeAddOrderPacket(6, 6, FeedSide::Buy, 106, 6)));
        assert(receiver.drainPackets());
        assert(receiver.expectedSequence() == 8 && "recovery must drain 5, 6, and buffered 7");

        for (std::uint32_t sequenceNumber = 8; sequenceNumber <= 10; ++sequenceNumber) {
            assert(receiver.bufferPacket(makeAddOrderPacket(sequenceNumber,sequenceNumber,FeedSide::Buy,100 + sequenceNumber,sequenceNumber)));
            assert(receiver.drainPackets());
        }

        assert(receiver.expectedSequence() == 11 && "all ten events must apply in sequence");
        assert(book.getBuyMap().size() == 10 && "all ten orders must exist in the local book");
        assert(book.getBuyMap().begin()->first == 110 && "best buy price must be 110");
        assert(book.getBuyMap().at(105).front().getOrderId() == 5 && "recovered order must apply");
        return true;
    }
};

int main() {
    MarketDataReceiverTest test;
    return test.runAllTests();
}
