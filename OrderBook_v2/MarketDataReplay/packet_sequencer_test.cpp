#include "packet_sequencer.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

class PacketSequencerTest {
public:
    int runAllTests() {
        bool allPassed = true;

        if (doesNotAdvanceForEmptyBuffer()) {
            std::cout << "PASS: doesNotAdvanceForEmptyBuffer\n";
        } else {
            std::cout << "FAIL: doesNotAdvanceForEmptyBuffer\n";
            allPassed = false;
        }

        if (drainsContiguousPackets()) {
            std::cout << "PASS: drainsContiguousPackets\n";
        } else {
            std::cout << "FAIL: drainsContiguousPackets\n";
            allPassed = false;
        }

        if (doesNotDrainFuturePacket()) {
            std::cout << "PASS: doesNotDrainFuturePacket\n";
        } else {
            std::cout << "FAIL: doesNotDrainFuturePacket\n";
            allPassed = false;
        }

        if (drainsPreviouslyBufferedPackets()) {
            std::cout << "PASS: drainsPreviouslyBufferedPackets\n";
        } else {
            std::cout << "FAIL: drainsPreviouslyBufferedPackets\n";
            allPassed = false;
        }

        if (doesNotDrainWrongSequenceInValidSlot()) {
            std::cout << "PASS: doesNotDrainWrongSequenceInValidSlot\n";
        } else {
            std::cout << "FAIL: doesNotDrainWrongSequenceInValidSlot\n";
            allPassed = false;
        }

        if (appliesMarketEventsInSequence()) {
            std::cout << "PASS: appliesMarketEventsInSequence\n";
        } else {
            std::cout << "FAIL: appliesMarketEventsInSequence\n";
            allPassed = false;
        }

        return allPassed ? 0 : 1;
    }

private:
    PendingPacketSlot makeValidSlot(std::uint32_t sequenceNumber) {
        AddOrderEvent event{
            sequenceNumber,
            FeedSide::Buy,
            100 + sequenceNumber,
            10
        };
        FeedPacket packet{
            sequenceNumber,
            MessageType::AddOrder,
            encodeAddOrderPayload(event)
        };
        return PendingPacketSlot{sequenceNumber, true, packet};
    }

    std::size_t bufferIndexFor(std::uint32_t sequenceNumber) {
        return (sequenceNumber - 1) & (BUFFERSIZE - 1);
    }

    bool doesNotAdvanceForEmptyBuffer() {
        std::vector<PendingPacketSlot> packetBuffer(BUFFERSIZE);
        std::uint32_t expected = 1;
        Orderbook book;
        MarketDataApplier applier(book);
        assert(drainPendingPackets(packetBuffer, expected, applier));
        assert(expected == 1 && "an empty buffer must not advance expected");
        return true;
    }

    bool drainsContiguousPackets() {
        std::vector<PendingPacketSlot> packetBuffer(BUFFERSIZE);
        std::uint32_t expected = 1;
        Orderbook book;
        MarketDataApplier applier(book);

        packetBuffer[bufferIndexFor(1)] = makeValidSlot(1);
        packetBuffer[bufferIndexFor(2)] = makeValidSlot(2);
        packetBuffer[bufferIndexFor(3)] = makeValidSlot(3);
        assert(drainPendingPackets(packetBuffer, expected, applier));
        assert(expected == 4 && "contiguous sequences 1 through 3 must drain");
        assert(!packetBuffer[bufferIndexFor(1)].valid && "drained slot must become invalid");
        assert(!packetBuffer[bufferIndexFor(2)].valid && "drained slot must become invalid");
        assert(!packetBuffer[bufferIndexFor(3)].valid && "drained slot must become invalid");
        return true;
    }

    bool doesNotDrainFuturePacket() {
        std::vector<PendingPacketSlot> packetBuffer(BUFFERSIZE);
        std::uint32_t expected = 1;
        Orderbook book;
        MarketDataApplier applier(book);

        packetBuffer[bufferIndexFor(3)] = makeValidSlot(3);
        assert(drainPendingPackets(packetBuffer, expected, applier));
        assert(expected == 1 && "a future packet must not advance expected");
        assert(packetBuffer[bufferIndexFor(3)].valid && "future packet must remain buffered");
        return true;
    }

    bool drainsPreviouslyBufferedPackets() {
        std::vector<PendingPacketSlot> packetBuffer(BUFFERSIZE);
        std::uint32_t expected = 1;
        Orderbook book;
        MarketDataApplier applier(book);

        packetBuffer[bufferIndexFor(3)] = makeValidSlot(3);
        assert(drainPendingPackets(packetBuffer, expected, applier));

        packetBuffer[bufferIndexFor(1)] = makeValidSlot(1);
        packetBuffer[bufferIndexFor(2)] = makeValidSlot(2);
        assert(drainPendingPackets(packetBuffer, expected, applier));
        assert(expected == 4 && "draining must continue into previously buffered packet 3");
        assert(!packetBuffer[bufferIndexFor(3)].valid && "packet 3 must be consumed");
        return true;
    }

    bool doesNotDrainWrongSequenceInValidSlot() {
        std::vector<PendingPacketSlot> packetBuffer(BUFFERSIZE);
        std::uint32_t expected = 1;
        Orderbook book;
        MarketDataApplier applier(book);

        packetBuffer[bufferIndexFor(1)] = makeValidSlot(2);
        assert(drainPendingPackets(packetBuffer, expected, applier));
        assert(expected == 1 && "a slot with the wrong sequence must not drain");
        assert(packetBuffer[bufferIndexFor(1)].valid && "wrong-sequence slot must remain valid");
        return true;
    }

    bool appliesMarketEventsInSequence() {
        std::vector<PendingPacketSlot> packetBuffer(BUFFERSIZE);
        std::uint32_t expected = 1;
        Orderbook book;
        MarketDataApplier applier(book);
        packetBuffer[bufferIndexFor(1)]=makeValidSlot(1);
        CancelOrderEvent cancelEvent{1};
        FeedPacket cancelPacket{2,MessageType::CancelOrder,encodeCancelOrderPayload(cancelEvent)};
        packetBuffer[bufferIndexFor(2)] = PendingPacketSlot{2, true, cancelPacket};
        assert(drainPendingPackets(packetBuffer, expected, applier));
        assert(expected == 3 && "add and cancel events must both be sequenced");
        assert(book.getBuyMap().empty() && "the cancel event must remove the added order");
        return true;
    }
};

int main() {
    PacketSequencerTest test;
    return test.runAllTests();
}
