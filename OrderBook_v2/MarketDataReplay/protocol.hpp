#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>

inline constexpr std::uint32_t FeedMagic = 0xFFFFFFFF;
inline constexpr std::uint16_t FeedVersion = 1;
inline constexpr std::size_t PacketHeaderSize = 14;
inline constexpr std::uint32_t MaxPacketLength = 1024;
inline constexpr std::size_t AddOrderPayloadSize = 25;
inline constexpr std::size_t CancelOrderPayloadSize = 8;
using PayloadLength = std::uint16_t;

enum class MessageType: std::uint16_t{
    Test = 1,
    AddOrder= 2,
    CancelOrder =3,
};

struct FeedPacket{
    std::uint32_t sequenceNumber{};
    MessageType messageType{};
    std::vector<std::uint8_t> payload{};
};


enum class FeedSide : std::uint8_t {
    Buy = 1,
    Sell = 2
};

struct AddOrderEvent {
    std::uint64_t orderId{};
    FeedSide side{};
    std::uint64_t price{};
    std::uint64_t quantity{};
};

struct CancelOrderEvent {
    std::uint64_t orderId{};
};

std::vector<std::uint8_t> encodePacket(const FeedPacket& packet);

bool decodePacket(const std::uint8_t* data, std::size_t length, FeedPacket &packet);
std::vector<std::uint8_t> encodeAddOrderPayload( const AddOrderEvent& event);
bool decodeAddOrderPayload(const std::vector<std::uint8_t>& payload,AddOrderEvent& event);
std::vector<std::uint8_t> encodeCancelOrderPayload( const CancelOrderEvent& event);
bool decodeCancelOrderPayload(const std::vector<std::uint8_t>& payload,CancelOrderEvent& event);

