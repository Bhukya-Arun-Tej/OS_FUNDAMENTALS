#include "Orderbook.hpp"

#include <chrono>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstddef>

using Clock = std::chrono::steady_clock;

constexpr int WarmupCount = 1'000;
constexpr int SampleCount = 1'000'000;
constexpr int BatchSize = 1'000;

void calcualteAddOrderLatency(){
    // Makes the order-book result observable in an optimized build. Update it only after each timer stops, so it is not part of the measured latency.
    volatile std::size_t benchmarkSink = 0;
    for(int i=0;i<WarmupCount;++i){
        Orderbook book;
        book.addOrder(1,Side::BUY,100,10);
        benchmarkSink += book.getBuyMap().size();
    }

    std::vector<long long> latencies;
    latencies.reserve(SampleCount);
    long long totalLatency =0ll;
    for(int i=0;i<SampleCount;i++){
        Orderbook book;
        const auto start = Clock::now();
        book.addOrder(1,Side::BUY,100,10);
        const auto end = Clock::now();
        benchmarkSink += book.getBuyMap().size();

        const auto latency = std::chrono::duration_cast<std::chrono::nanoseconds> (end-start).count();
        latencies.push_back(latency);
        totalLatency+=latency;

    }
    std::cout<<"Recorded samples: "<<latencies.size()<<"\n";

    const auto averageLatency = static_cast<double> (totalLatency) / latencies.size();
    std::cout<<"average addOrder latency: "<<averageLatency<<" ns\n";

    std::sort(latencies.begin(),latencies.end());
    long long minLatency = latencies[0];
    long long maxLatency = latencies.back();

    std::cout<<"minimum addOrder latency: "<<minLatency<<" ns\n";
    std::cout<<"maximum addOrder latency: "<<maxLatency<<" ns\n";

    const auto middle = latencies.size()/2;
    const double medianLatency = (static_cast<double> (latencies[middle-1]) + static_cast<double> (latencies[middle])) /2.0;
    std::cout<<"median addOrder latency: "<<medianLatency<<" ns\n";

    const auto p99Index = ((latencies.size())*99)/100 -1 ;
    const auto p999Index = ((latencies.size())*999)/1000 -1 ;

    const auto p99Latency = latencies[p99Index];
    const auto p999Latency = latencies[p999Index];

    std::cout<<"p99 addOrder latency: "<<p99Latency<<" ns\n";
    std::cout<<"p99.9 addOrder latency: "<<p999Latency<<" ns\n";


    std::vector<Orderbook> books(BatchSize);
    const auto batchStart = Clock::now();
    for(int i=0;i<BatchSize;i++){
        books[i].addOrder(1, Side::BUY,100,10);
    }
    const auto batchEnd = Clock::now();

    for(int i=0;i<BatchSize;i++){
        benchmarkSink+= books[i].getBuyMap().size();
    }
    const auto batchTotalLatency = std::chrono::duration_cast<std::chrono::nanoseconds>(batchEnd - batchStart).count();
    const double batchAverageLatency = static_cast<double>(batchTotalLatency) / BatchSize;

    std::cout<<"batched total latency: "<< batchTotalLatency<<" ns\n";
    std::cout<<"batched average latency: "<<batchAverageLatency<<" ns\n";

    std::cout << "benchmark sink: " << benchmarkSink << '\n';

}

void calculateFullFillLatency(){
    // Makes the order-book result observable in an optimized build. Update it only after each timer stops, so it is not part of the measured latency.
    volatile std::size_t benchmarkSink = 0;
    for(int i=0;i<WarmupCount;++i){
        Orderbook book;
        book.addOrder(1,Side::BUY,100,10);
        Trades trades = book.addOrder(2,Side::SELL,100,10);
        benchmarkSink += trades.size();
    }

    std::vector<long long> latencies;
    latencies.reserve(SampleCount);
    long long totalLatency =0ll;
    for(int i=0;i<SampleCount;i++){
        Orderbook book;
        book.addOrder(1,Side::BUY,100,10);
        const auto start = Clock::now();
        Trades trades = book.addOrder(2,Side::SELL,100,10);
        const auto end = Clock::now();
        benchmarkSink += trades.size();

        const auto latency = std::chrono::duration_cast<std::chrono::nanoseconds> (end-start).count();
        latencies.push_back(latency);
        totalLatency+=latency;

    }
    std::cout<<"Recorded samples: "<<latencies.size()<<"\n";

    const auto averageLatency = static_cast<double> (totalLatency) / latencies.size();
    std::cout<<"average fullFill latency: "<<averageLatency<<" ns\n";

    std::sort(latencies.begin(),latencies.end());
    long long minLatency = latencies[0];
    long long maxLatency = latencies.back();

    std::cout<<"minimum fullFill latency: "<<minLatency<<" ns\n";
    std::cout<<"maximum fullFill latency: "<<maxLatency<<" ns\n";

    const auto middle = latencies.size()/2;
    const double medianLatency = (static_cast<double> (latencies[middle-1]) + static_cast<double> (latencies[middle])) /2.0;
    std::cout<<"median fullFill latency: "<<medianLatency<<" ns\n";

    const auto p99Index = ((latencies.size())*99)/100 -1 ;
    const auto p999Index = ((latencies.size())*999)/1000 -1 ;

    const auto p99Latency = latencies[p99Index];
    const auto p999Latency = latencies[p999Index];

    std::cout<<"p99 fullFill latency: "<<p99Latency<<" ns\n";
    std::cout<<"p99.9 fullFill latency: "<<p999Latency<<" ns\n";


    std::vector<Orderbook> books(BatchSize);
    for(int i=0;i<BatchSize;i++){
        books[i].addOrder(1, Side::BUY,100,10);
    }
    const auto batchStart = Clock::now();
    for(int i=0;i<BatchSize;i++){
        books[i].addOrder(2, Side::SELL,100,10);
    }
    const auto batchEnd = Clock::now();

    for(int i=0;i<BatchSize;i++){
        benchmarkSink+= books[i].getBuyMap().empty() && books[i].getSellMap().empty();
    }
    
    const auto batchTotalLatency = std::chrono::duration_cast<std::chrono::nanoseconds>(batchEnd - batchStart).count();
    const double batchAverageLatency = static_cast<double>(batchTotalLatency) / BatchSize;

    std::cout<<"batched total latency: "<< batchTotalLatency<<" ns\n";
    std::cout<<"batched average latency: "<<batchAverageLatency<<" ns\n";

    std::cout << "benchmark sink: " << benchmarkSink << '\n';
}

int main(){
    std::cout<<"-------addOrder Latency-------";
    calcualteAddOrderLatency();
    std::cout<<"-------addOrder Latency------- \n\n";


    std::cout<<"-------full fill Latency-------";
    calculateFullFillLatency();
    std::cout<<"-------full fill Latency------- \n\n";

    return 0;
}
