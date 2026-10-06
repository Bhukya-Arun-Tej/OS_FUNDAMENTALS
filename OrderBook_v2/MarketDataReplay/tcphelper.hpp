#pragma once

#include <cstdint>
#include <cstddef>

bool sendAll(int socketFd, const std::uint8_t* data, std::size_t length);
bool receiveAll(int socketFd,std::uint8_t* data, std::size_t length);