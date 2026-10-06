#pragma once

#include <vector>
#include <cstdint>
#include "protocol.hpp"

bool recoverMissingPackets(std::uint32_t firstMissingSequence, std::uint32_t lastMissingSequence, std::vector<FeedPacket>& recoveredPackets);