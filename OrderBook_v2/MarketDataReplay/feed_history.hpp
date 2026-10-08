#pragma once
 
#include <cstdint>
#include <vector>
#include <cstddef>
#include <mutex>


//Stored encoded packet bytes for TCP recovery
struct StoredPacket
{
    std::uint32_t sequenceNumber{};
    std::vector<std::uint8_t> bytes{};
};

class FeedHistory{
    private:
        std::vector<StoredPacket>m_history;
        std::size_t m_capacity;
        mutable std::mutex m_mutex;

    public:
        explicit FeedHistory(std::size_t capacity);
        void store(std::uint32_t sequenceNumber, const std::vector<std::uint8_t>& bytes);
        bool find(std::uint32_t sequenceNumber, std::vector<std::uint8_t>& result)const;
};

