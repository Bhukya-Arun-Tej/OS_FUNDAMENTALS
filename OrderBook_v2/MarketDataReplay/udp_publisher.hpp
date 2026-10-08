#pragma once

#include <cstdint>
#include <vector>

bool publishBytesUDP(int socketFd, const std::vector<std::uint8_t>& encodedBytes);