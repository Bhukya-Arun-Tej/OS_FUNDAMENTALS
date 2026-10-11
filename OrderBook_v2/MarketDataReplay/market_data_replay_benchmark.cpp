#include "protocol.hpp"
#include "market_data_applier.hpp"
#include "market_data_receiver.hpp"
#include "../Orderbook.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

using Clock = std::chrono::steady_clock;
constexpr std::size_t SampleCount = 1'000'000;

std::mt19937_64 generator(42);

void populateAddOnlyReplayPackets(std::vector<std::vector<std::uint8_t>>& replayPackets){
    replayPackets.reserve(SampleCount);

    for(std::uint32_t sequence=1;sequence<=SampleCount;sequence++){
        AddOrderEvent event{sequence, FeedSide::Buy, (100+sequence)%1000 +1,(100+sequence)%1000 +1};
        const FeedPacket packet {sequence, MessageType::AddOrder, encodeAddOrderPayload(event)};
        replayPackets.push_back(encodePacket(packet));
    }
}

void populateCancelOnlyReplayPackets(std::vector<std::vector<std::uint8_t>>& replayPackets,MarketDataApplier& applier){
    replayPackets.reserve(SampleCount);

    for(std::uint32_t sequence=1;sequence<=SampleCount;sequence++){
        AddOrderEvent addEvent{sequence, FeedSide::Buy, (100+sequence)%1000 +1,(100+sequence)%1000 +1};
        const FeedPacket addPacket {sequence, MessageType::AddOrder, encodeAddOrderPayload(addEvent)};
        applier.processPacket(addPacket);
        CancelOrderEvent event{sequence};
        const FeedPacket packet {sequence, MessageType::CancelOrder, encodeCancelOrderPayload(event)};
        replayPackets.push_back(encodePacket(packet));
    }    
}

void populateMixReplayPackets(std::vector<std::vector<std::uint8_t>>& replayPackets,MarketDataApplier& applier){
    replayPackets.reserve(SampleCount);

    for(std::uint32_t sequence=1;sequence<=700'000;sequence++){
        AddOrderEvent addEvent{sequence, FeedSide::Buy, (100+sequence)%1000 +1,(100+sequence)%1000 +1};
        const FeedPacket addPacket {sequence, MessageType::AddOrder, encodeAddOrderPayload(addEvent)};
        replayPackets.push_back(encodePacket(addPacket));
    }

    for(std::uint32_t sequence=700'001;sequence<=SampleCount;sequence++){
        AddOrderEvent addEvent{sequence, FeedSide::Buy, (100+sequence)%1000 +1,(100+sequence)%1000 +1};
        const FeedPacket addPacket {sequence, MessageType::AddOrder, encodeAddOrderPayload(addEvent)};
        applier.processPacket(addPacket);
        CancelOrderEvent event{sequence};
        const FeedPacket packet {sequence, MessageType::CancelOrder, encodeCancelOrderPayload(event)};
        replayPackets.push_back(encodePacket(packet));
    }
    
}


void printLatencyStats(const char* benchmarkName, std::vector<std::int64_t>& latencies, uint64_t totalLatency){
    std::cout<<"Recorded samples: "<<latencies.size()<<"\n";

    const auto averageLatency = static_cast<double> (totalLatency) / latencies.size();
    std::cout<<"average "<<benchmarkName<<" latency: "<<averageLatency<<" ns\n";

    std::sort(latencies.begin(),latencies.end());
    long long minLatency = latencies[0];
    long long maxLatency = latencies.back();

    std::cout<<"minimum "<<benchmarkName<<" latency: "<<minLatency<<" ns\n";
    std::cout<<"maximum "<<benchmarkName<<" latency: "<<maxLatency<<" ns\n";

    const auto middle = latencies.size()/2;
    const double medianLatency = (static_cast<double> (latencies[middle-1]) + static_cast<double> (latencies[middle])) /2.0;
    std::cout<<"median "<<benchmarkName<<" latency: "<<medianLatency<<" ns\n";

    const auto p99Index = ((latencies.size())*99)/100 -1 ;
    const auto p999Index = ((latencies.size())*999)/1000 -1 ;

    const auto p99Latency = latencies[p99Index];
    const auto p999Latency = latencies[p999Index];

    std::cout<<"p99 "<<benchmarkName<<" latency: "<<p99Latency<<" ns\n";
    std::cout<<"p99.9 "<<benchmarkName<<" latency: "<<p999Latency<<" ns\n";
}

void calculateAddOnlyLatency(){
    Orderbook book;
    MarketDataApplier applier(book);
    MarketDataReceiver receiver(applier);

    std::vector<std::int64_t> latencies;
    uint64_t totalLatency = 0;

    latencies.reserve(SampleCount);
    volatile std::size_t benchmarkSink = 0;
    std::vector<std::vector<std::uint8_t>> replayPackets;
    populateAddOnlyReplayPackets(replayPackets);
    for (const auto& encodedBytes : replayPackets) {
        FeedPacket packet;
        const auto start = Clock::now();
        const bool decoded = decodePacket(encodedBytes.data(),encodedBytes.size(),packet);
        const bool buffered = decoded && receiver.bufferPacket(packet);
        const bool drained = buffered && receiver.drainPackets();
        const auto end = Clock::now();
        if (!drained) {
            std::cerr << "Replay packet failed\n";
            return;
        }
        benchmarkSink += receiver.expectedSequence();
        const auto latency =std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        latencies.push_back(latency);
        totalLatency+=latency;
    }
    printLatencyStats("client processing", latencies ,totalLatency);
    std::cout<<"benchmark sink: "<<benchmarkSink<<"\n";
}

void calculateCancelOnlyLatency(){
    Orderbook book;
    MarketDataApplier applier(book);
    MarketDataReceiver receiver(applier);

    std::vector<std::int64_t> latencies;
    uint64_t totalLatency = 0;

    latencies.reserve(SampleCount);
    volatile std::size_t benchmarkSink = 0;
    std::vector<std::vector<std::uint8_t>> replayPackets;
    populateCancelOnlyReplayPackets(replayPackets, applier);
    for (const auto& encodedBytes : replayPackets) {
        FeedPacket packet;
        const auto start = Clock::now();
        const bool decoded = decodePacket(encodedBytes.data(),encodedBytes.size(),packet);
        const bool buffered = decoded && receiver.bufferPacket(packet);
        const bool drained = buffered && receiver.drainPackets();
        const auto end = Clock::now();
        if (!drained) {
            std::cerr << "Replay packet failed\n";
            return;
        }
        benchmarkSink += receiver.expectedSequence();
        const auto latency =std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        latencies.push_back(latency);
        totalLatency+=latency;
    }
    printLatencyStats("client processing", latencies ,totalLatency);
    std::cout<<"benchmark sink: "<<benchmarkSink<<"\n";
}


void calculateMixedLatency(){
    Orderbook book;
    MarketDataApplier applier(book);
    MarketDataReceiver receiver(applier);

    std::vector<std::int64_t> latencies;
    uint64_t totalLatency = 0;

    latencies.reserve(SampleCount);
    volatile std::size_t benchmarkSink = 0;
    std::vector<std::vector<std::uint8_t>> replayPackets;
    populateMixReplayPackets(replayPackets, applier);
    for (const auto& encodedBytes : replayPackets) {
        FeedPacket packet;
        const auto start = Clock::now();
        const bool decoded = decodePacket(encodedBytes.data(),encodedBytes.size(),packet);
        const bool buffered = decoded && receiver.bufferPacket(packet);
        const bool drained = buffered && receiver.drainPackets();
        const auto end = Clock::now();
        if (!drained) {
            std::cerr << "Replay packet failed\n";
            return;
        }
        benchmarkSink += receiver.expectedSequence();
        const auto latency =std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        latencies.push_back(latency);
        totalLatency+=latency;
    }
    printLatencyStats("client processing", latencies ,totalLatency);
    std::cout<<"benchmark sink: "<<benchmarkSink<<"\n";
}


int main(){

    std::cout<<"-------addOnlyReplay Latency-------";
    calculateAddOnlyLatency();
    std::cout<<"-------addOnlyReplay Latency------- \n\n";


    std::cout<<"-------cancelOnlyReplay Latency-------";
    calculateCancelOnlyLatency();
    std::cout<<"-------cancelOnlyReplay Latency------- \n\n";

    std::cout<<"-------mixedReplay Latency-------";
    calculateMixedLatency();
    std::cout<<"-------mixedReplay Latency------- \n\n";


    return 0;
}