#include "protocol.hpp"

#include <cassert>
#include <iostream>
#include <vector>

class ProtocolTest {
public:
    int runAllTests() {
        bool allPassed = true;

        if (encodePacketWritesExpectedBytes()) {
            std::cout << "PASS: encodePacketWritesExpectedBytes\n";
        } else {
            std::cout << "FAIL: encodePacketWritesExpectedBytes\n";
            allPassed = false;
        }

        if (decodePacketReadsExpectedBytes()) {
            std::cout << "PASS: decodePacketReadsExpectedBytes\n";
        } else {
            std::cout << "FAIL: decodePacketReadsExpectedBytes\n";
            allPassed = false;
        }

        if (decodePacketRejectsShortHeader()) {
            std::cout << "PASS: decodePacketRejectsShortHeader\n";
        } else {
            std::cout << "FAIL: decodePacketRejectsShortHeader\n";
            allPassed = false;
        }

        if (decodePacketRejectsIncorrectMagic()) {
            std::cout << "PASS: decodePacketRejectsIncorrectMagic\n";
        } else {
            std::cout << "FAIL: decodePacketRejectsIncorrectMagic\n";
            allPassed = false;
        }

        if (decodePacketRejectsUnsupportedVersion()) {
            std::cout << "PASS: decodePacketRejectsUnsupportedVersion\n";
        } else {
            std::cout << "FAIL: decodePacketRejectsUnsupportedVersion\n";
            allPassed = false;
        }

        if (decodePacketRejectsUnknownMessageType()) {
            std::cout << "PASS: decodePacketRejectsUnknownMessageType\n";
        } else {
            std::cout << "FAIL: decodePacketRejectsUnknownMessageType\n";
            allPassed = false;
        }

        if (decodePacketRejectsIncorrectPayloadLength()) {
            std::cout << "PASS: decodePacketRejectsIncorrectPayloadLength\n";
        } else {
            std::cout << "FAIL: decodePacketRejectsIncorrectPayloadLength\n";
            allPassed = false;
        }

        return allPassed ? 0 : 1;
    }

private:
    bool encodePacketWritesExpectedBytes() {
        FeedPacket packet{42, MessageType::Test, "hello"};

        const std::vector<std::uint8_t> expected{
            0xFF, 0xFF, 0xFF, 0xFF,
            0x00, 0x01,
            0x00, 0x01,
            0x00, 0x00, 0x00, 0x2A,
            0x00, 0x05,
            0x68, 0x65, 0x6C, 0x6C, 0x6F
        };

        const std::vector<std::uint8_t> encoded = encodePacket(packet);
        assert(encoded == expected && "encoded packet bytes should match the protocol layout");
        return true;
    }

    bool decodePacketReadsExpectedBytes() {
        const std::vector<std::uint8_t> bytes{
            0xFF, 0xFF, 0xFF, 0xFF,
            0x00, 0x01,
            0x00, 0x01,
            0x00, 0x00, 0x00, 0x2A,
            0x00, 0x05,
            0x68, 0x65, 0x6C, 0x6C, 0x6F
        };

        FeedPacket packet;
        const bool decoded = decodePacket(bytes.data(), bytes.size(), packet);
        assert(decoded && "valid packet bytes should decode");
        assert(packet.sequenceNumber == 42 && "sequence number should be 42");
        assert(packet.messageType == MessageType::Test && "message type should be Test");
        assert(packet.payload == "hello" && "payload should be hello");

        return true;
    }

    bool decodePacketRejectsShortHeader() {
        const std::vector<std::uint8_t> bytes(PacketHeaderSize - 1, 0);

        FeedPacket packet;
        const bool decoded = decodePacket(bytes.data(), bytes.size(), packet);
        assert(!decoded && "packet shorter than its header should be rejected");
        return true;
    }

    bool decodePacketRejectsIncorrectMagic() {
        const std::vector<std::uint8_t> bytes{
            0xFE, 0xFF, 0xFF, 0xFF,
            0x00, 0x01,
            0x00, 0x01,
            0x00, 0x00, 0x00, 0x2A,
            0x00, 0x05,
            0x68, 0x65, 0x6C, 0x6C, 0x6F
        };

        FeedPacket packet;
        const bool decoded = decodePacket(bytes.data(), bytes.size(), packet);
        assert(!decoded && "packet with an incorrect magic number should be rejected");
        return true;
    }

    bool decodePacketRejectsUnsupportedVersion() {
        const std::vector<std::uint8_t> bytes{
            0xFF, 0xFF, 0xFF, 0xFF,
            0x00, 0x02,
            0x00, 0x01,
            0x00, 0x00, 0x00, 0x2A,
            0x00, 0x05,
            0x68, 0x65, 0x6C, 0x6C, 0x6F
        };

        FeedPacket packet;
        const bool decoded = decodePacket(bytes.data(), bytes.size(), packet);
        assert(!decoded && "packet with an unsupported version should be rejected");
        return true;
    }

    bool decodePacketRejectsUnknownMessageType() {
        const std::vector<std::uint8_t> bytes{
            0xFF, 0xFF, 0xFF, 0xFF,
            0x00, 0x01,
            0x00, 0x02,
            0x00, 0x00, 0x00, 0x2A,
            0x00, 0x05,
            0x68, 0x65, 0x6C, 0x6C, 0x6F
        };

        FeedPacket packet;
        const bool decoded = decodePacket(bytes.data(), bytes.size(), packet);
        assert(!decoded && "packet with an unknown message type should be rejected");
        return true;
    }

    bool decodePacketRejectsIncorrectPayloadLength() {
        const std::vector<std::uint8_t> bytes{
            0xFF, 0xFF, 0xFF, 0xFF,
            0x00, 0x01,
            0x00, 0x01,
            0x00, 0x00, 0x00, 0x2A,
            0x00, 0x06,
            0x68, 0x65, 0x6C, 0x6C, 0x6F
        };

        FeedPacket packet;
        const bool decoded = decodePacket(bytes.data(), bytes.size(), packet);
        assert(!decoded && "packet with an incorrect payload length should be rejected");
        return true;
    }
};

int main() {
    ProtocolTest test;
    return test.runAllTests();
}
