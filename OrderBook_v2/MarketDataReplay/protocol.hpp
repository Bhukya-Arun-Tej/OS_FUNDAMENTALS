#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>

inline constexpr std::uint32_t FeedMagic = 0xFFFFFFFF;
inline constexpr std::uint16_t FeedVersion = 1;
inline constexpr std::size_t PacketHeaderSize = 14;
inline constexpr std::uint32_t MaxPacketLength = 1024;
using PayloadLength = std::uint16_t;

enum class MessageType: std::uint16_t{
    Test = 1
};

struct FeedPacket{
    std::uint32_t sequenceNumber{};
    MessageType messageType{};
    std::string payload{};
};

std::vector<std::uint8_t> encodePacket(const FeedPacket& packet);

bool decodePacket(const std::uint8_t* data, std::size_t length, FeedPacket &packet);

