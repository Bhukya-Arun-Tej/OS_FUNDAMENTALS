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
constexpr OrderId DeepBookOrderCount = 1'000'000;
constexpr Price DeepBookBestBid = 10'000;
constexpr Price DeepBookPriceLevelCount = 1'000;
constexpr Price DeepBookLowestBid = DeepBookBestBid-DeepBookPriceLevelCount+1;
constexpr OrderId DeepBookWarmupCount = 1'000;
constexpr OrderId DeepBookSampleCount = 100'000;

// Sorts the recorded individual samples and prints their latency statistics.
void printLatencyStats(const char* benchmarkName, std::vector<long long>& latencies, long long totalLatency){
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
    printLatencyStats("addOrder",latencies,totalLatency);

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
    printLatencyStats("fullFill",latencies,totalLatency);

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

void calculatePartialRestingFillLatency(){
    // Makes the order-book result observable in an optimized build. Update it only after each timer stops, so it is not part of the measured latency.
    volatile std::size_t benchmarkSink = 0;
    for(int i=0;i<WarmupCount;++i){
        Orderbook book;
        book.addOrder(1,Side::BUY,100,20);
        Trades trades = book.addOrder(2,Side::SELL,100,10);
        benchmarkSink += trades.size()==1 && book.getSellMap().empty() && book.getBuyMap().size()==1 && book.getBuyMap().begin()->second.front().getRemainingQuantity()==10;
    }

    std::vector<long long> latencies;
    latencies.reserve(SampleCount);
    long long totalLatency =0ll;
    for(int i=0;i<SampleCount;i++){
        Orderbook book;
        book.addOrder(1,Side::BUY,100,20);
        const auto start = Clock::now();
        Trades trades = book.addOrder(2,Side::SELL,100,10);
        const auto end = Clock::now();
        benchmarkSink += trades.size()==1 && book.getSellMap().empty() && book.getBuyMap().size()==1 && book.getBuyMap().begin()->second.front().getRemainingQuantity()==10;

        const auto latency = std::chrono::duration_cast<std::chrono::nanoseconds> (end-start).count();
        latencies.push_back(latency);
        totalLatency+=latency;
    }
    printLatencyStats("partial resting fill",latencies,totalLatency);
    std::vector<Orderbook> books(BatchSize);
    for(int i=0;i<BatchSize;i++){
        books[i].addOrder(1, Side::BUY,100,20);
    }
    const auto batchStart = Clock::now();
    for(int i=0;i<BatchSize;i++){
        books[i].addOrder(2, Side::SELL,100,10);
    }
    const auto batchEnd = Clock::now();

    for(int i=0;i<BatchSize;i++){
        benchmarkSink += books[i].getSellMap().empty() && books[i].getBuyMap().size()==1 && books[i].getBuyMap().begin()->second.front().getRemainingQuantity()==10;
    }

    const auto batchTotalLatency = std::chrono::duration_cast<std::chrono::nanoseconds>(batchEnd - batchStart).count();
    const double batchAverageLatency = static_cast<double>(batchTotalLatency) / BatchSize;

    std::cout<<"batched total latency: "<< batchTotalLatency<<" ns\n";
    std::cout<<"batched average latency: "<<batchAverageLatency<<" ns\n";

    std::cout << "benchmark sink: " << benchmarkSink << '\n';
}

void calculatePartialIncomingFillLatency(){
    // Makes the order-book result observable in an optimized build. Update it only after each timer stops, so it is not part of the measured latency.
    volatile std::size_t benchmarkSink = 0;
    for(int i=0;i<WarmupCount;++i){
        Orderbook book;
        book.addOrder(1,Side::BUY,100,10);
        Trades trades = book.addOrder(2,Side::SELL,100,20);
        benchmarkSink += trades.size()==1 && book.getBuyMap().empty() && book.getSellMap().size()==1 && book.getSellMap().begin()->second.front().getRemainingQuantity()==10;
    }

    std::vector<long long> latencies;
    latencies.reserve(SampleCount);
    long long totalLatency =0ll;
    for(int i=0;i<SampleCount;i++){
        Orderbook book;
        book.addOrder(1,Side::BUY,100,10);
        const auto start = Clock::now();
        Trades trades = book.addOrder(2,Side::SELL,100,20);
        const auto end = Clock::now();
        benchmarkSink += trades.size()==1 && book.getBuyMap().empty() && book.getSellMap().size()==1 && book.getSellMap().begin()->second.front().getRemainingQuantity()==10;

        const auto latency = std::chrono::duration_cast<std::chrono::nanoseconds> (end-start).count();
        latencies.push_back(latency);
        totalLatency+=latency;
    }
    printLatencyStats("partial incoming fill",latencies,totalLatency);
    std::vector<Orderbook> books(BatchSize);
    for(int i=0;i<BatchSize;i++){
        books[i].addOrder(1, Side::BUY,100,10);
    }
    const auto batchStart = Clock::now();
    for(int i=0;i<BatchSize;i++){
        books[i].addOrder(2, Side::SELL,100,20);
    }
    const auto batchEnd = Clock::now();

    for(int i=0;i<BatchSize;i++){
        benchmarkSink += books[i].getBuyMap().empty() && books[i].getSellMap().size()==1 && books[i].getSellMap().begin()->second.front().getRemainingQuantity()==10;
    }

    const auto batchTotalLatency = std::chrono::duration_cast<std::chrono::nanoseconds>(batchEnd - batchStart).count();
    const double batchAverageLatency = static_cast<double>(batchTotalLatency) / BatchSize;

    std::cout<<"batched total latency: "<< batchTotalLatency<<" ns\n";
    std::cout<<"batched average latency: "<<batchAverageLatency<<" ns\n";

    std::cout << "benchmark sink: " << benchmarkSink << '\n';
}

void calculateMultiLevelSweepLatency(){
    // Makes the order-book result observable in an optimized build. Update it only after each timer stops, so it is not part of the measured latency.
    volatile std::size_t benchmarkSink = 0;
    for(int i=0;i<WarmupCount;++i){
        Orderbook book;
        book.addOrder(1,Side::BUY,102,10);
        book.addOrder(2,Side::BUY,101,10);
        book.addOrder(3,Side::BUY,100,10);
        Trades trades = book.addOrder(4,Side::SELL,100,25);
        benchmarkSink += trades.size()==3 && book.getSellMap().empty() && book.getBuyMap().size()==1 && book.getBuyMap().begin()->first==100 && book.getBuyMap().begin()->second.front().getOrderId()==3 && book.getBuyMap().begin()->second.front().getRemainingQuantity()==5;
    }

    std::vector<long long> latencies;
    latencies.reserve(SampleCount);
    long long totalLatency =0ll;
    for(int i=0;i<SampleCount;i++){
        Orderbook book;
        book.addOrder(1,Side::BUY,102,10);
        book.addOrder(2,Side::BUY,101,10);
        book.addOrder(3,Side::BUY,100,10);
        const auto start = Clock::now();
        Trades trades = book.addOrder(4,Side::SELL,100,25);
        const auto end = Clock::now();
        benchmarkSink += trades.size()==3 && book.getSellMap().empty() && book.getBuyMap().size()==1 && book.getBuyMap().begin()->first==100 && book.getBuyMap().begin()->second.front().getOrderId()==3 && book.getBuyMap().begin()->second.front().getRemainingQuantity()==5;

        const auto latency = std::chrono::duration_cast<std::chrono::nanoseconds> (end-start).count();
        latencies.push_back(latency);
        totalLatency+=latency;
    }
    printLatencyStats("multi level sweep",latencies,totalLatency);
    std::vector<Orderbook> books(BatchSize);
    for(int i=0;i<BatchSize;i++){
        books[i].addOrder(1, Side::BUY,102,10);
        books[i].addOrder(2, Side::BUY,101,10);
        books[i].addOrder(3, Side::BUY,100,10);
    }
    const auto batchStart = Clock::now();
    for(int i=0;i<BatchSize;i++){
        books[i].addOrder(4, Side::SELL,100,25);
    }
    const auto batchEnd = Clock::now();

    for(int i=0;i<BatchSize;i++){
        benchmarkSink += books[i].getSellMap().empty() && books[i].getBuyMap().size()==1 && books[i].getBuyMap().begin()->first==100 && books[i].getBuyMap().begin()->second.front().getOrderId()==3 && books[i].getBuyMap().begin()->second.front().getRemainingQuantity()==5;
    }

    const auto batchTotalLatency = std::chrono::duration_cast<std::chrono::nanoseconds>(batchEnd - batchStart).count();
    const double batchAverageLatency = static_cast<double>(batchTotalLatency) / BatchSize;

    std::cout<<"batched total latency: "<< batchTotalLatency<<" ns\n";
    std::cout<<"batched average latency: "<<batchAverageLatency<<" ns\n";

    std::cout << "benchmark sink: " << benchmarkSink << '\n';
}

void populateDeepBook(Orderbook& book){
    for(OrderId id=1;id<=DeepBookOrderCount;++id){
        const Price price = DeepBookBestBid - (id % DeepBookPriceLevelCount);
        book.addOrder(id,Side::BUY,price,10);
    }
}

std::size_t countRestingBuyOrders(const Orderbook& book){
    std::size_t totalRestingOrders = 0;
    for(const auto& priceLevel:book.getBuyMap()){
        totalRestingOrders += priceLevel.second.size();
    }
    return totalRestingOrders;
}

void calculateDeepBookBuildLatency(){
    Orderbook book;

    const auto start = Clock::now();
    populateDeepBook(book);
    const auto end = Clock::now();

    const auto totalLatency = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
    const double totalSeconds = static_cast<double>(totalLatency) / 1'000'000'000.0;
    const double ordersPerSecond = static_cast<double>(DeepBookOrderCount) / totalSeconds;

    const std::size_t totalRestingOrders = countRestingBuyOrders(book);

    std::cout<<"deep book orders: "<<totalRestingOrders<<"\n";
    std::cout<<"deep book price levels: "<<book.getBuyMap().size()<<"\n";
    std::cout<<"deep book build latency: "<<totalLatency<<" ns\n";
    std::cout<<"deep book build throughput: "<<ordersPerSecond<<" orders/second\n";
}

void calculateDeepBookCancelLatency(){
    Orderbook book;
    populateDeepBook(book);

    volatile std::size_t benchmarkSink = 0;
    for(OrderId id=1;id<=DeepBookWarmupCount;++id){
        book.cancelOrder(id);
        benchmarkSink += book.getBuyMap().size();
    }

    std::vector<long long> latencies;
    latencies.reserve(DeepBookSampleCount);
    long long totalLatency =0ll;
    for(OrderId id=DeepBookWarmupCount+1;id<=DeepBookWarmupCount+DeepBookSampleCount;++id){
        const auto start = Clock::now();
        book.cancelOrder(id);
        const auto end = Clock::now();
        benchmarkSink += book.getBuyMap().size();

        const auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
        latencies.push_back(latency);
        totalLatency+=latency;
    }
    printLatencyStats("deep book cancel",latencies,totalLatency);

    const std::size_t totalRestingOrders = countRestingBuyOrders(book);
    const OrderId expectedRestingOrders = DeepBookOrderCount-DeepBookWarmupCount-DeepBookSampleCount;
    std::cout<<"deep book resting orders after cancel: "<<totalRestingOrders<<"\n";
    std::cout<<"expected resting orders after cancel: "<<expectedRestingOrders<<"\n";
    std::cout<<"benchmark sink: "<<benchmarkSink<<"\n";
}

void calculateDeepBookAddLatency(){
    Orderbook book;
    populateDeepBook(book);

    volatile std::size_t benchmarkSink = 0;
    const OrderId firstNewOrderId = DeepBookOrderCount+1;
    for(OrderId id=firstNewOrderId;id<firstNewOrderId+DeepBookWarmupCount;++id){
        book.addOrder(id,Side::BUY,DeepBookBestBid,10);
        benchmarkSink += book.getBuyMap().size();
    }

    std::vector<long long> latencies;
    latencies.reserve(DeepBookSampleCount);
    long long totalLatency =0ll;
    for(OrderId id=firstNewOrderId+DeepBookWarmupCount;id<firstNewOrderId+DeepBookWarmupCount+DeepBookSampleCount;++id){
        const auto start = Clock::now();
        book.addOrder(id,Side::BUY,DeepBookBestBid,10);
        const auto end = Clock::now();
        benchmarkSink += book.getBuyMap().size();

        const auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
        latencies.push_back(latency);
        totalLatency+=latency;
    }
    printLatencyStats("deep book add",latencies,totalLatency);

    const std::size_t totalRestingOrders = countRestingBuyOrders(book);
    const OrderId expectedRestingOrders = DeepBookOrderCount+DeepBookWarmupCount+DeepBookSampleCount;
    std::cout<<"deep book resting orders after add: "<<totalRestingOrders<<"\n";
    std::cout<<"expected resting orders after add: "<<expectedRestingOrders<<"\n";
    std::cout<<"benchmark sink: "<<benchmarkSink<<"\n";
}

void calculateDeepBookMatchLatency(){
    Orderbook book;
    populateDeepBook(book);

    volatile std::size_t benchmarkSink = 0;
    const OrderId firstIncomingOrderId = DeepBookOrderCount+1;
    for(OrderId id=firstIncomingOrderId;id<firstIncomingOrderId+DeepBookWarmupCount;++id){
        Trades trades = book.addOrder(id,Side::SELL,DeepBookLowestBid,10);
        benchmarkSink += trades.size();
    }

    std::vector<long long> latencies;
    latencies.reserve(DeepBookSampleCount);
    long long totalLatency =0ll;
    for(OrderId id=firstIncomingOrderId+DeepBookWarmupCount;id<firstIncomingOrderId+DeepBookWarmupCount+DeepBookSampleCount;++id){
        const auto start = Clock::now();
        Trades trades = book.addOrder(id,Side::SELL,DeepBookLowestBid,10);
        const auto end = Clock::now();
        benchmarkSink += trades.size();

        const auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
        latencies.push_back(latency);
        totalLatency+=latency;
    }
    printLatencyStats("deep book match",latencies,totalLatency);

    const std::size_t totalRestingOrders = countRestingBuyOrders(book);
    const OrderId expectedRestingOrders = DeepBookOrderCount-DeepBookWarmupCount-DeepBookSampleCount;
    std::cout<<"deep book resting orders after match: "<<totalRestingOrders<<"\n";
    std::cout<<"expected resting orders after match: "<<expectedRestingOrders<<"\n";
    std::cout<<"benchmark sink: "<<benchmarkSink<<"\n";
}

int main(){
    std::cout<<"-------addOrder Latency-------";
    calcualteAddOrderLatency();
    std::cout<<"-------addOrder Latency------- \n\n";


    std::cout<<"-------full fill Latency-------";
    calculateFullFillLatency();
    std::cout<<"-------full fill Latency------- \n\n";

    std::cout<<"-------partial resting fill Latency-------";
    calculatePartialRestingFillLatency();
    std::cout<<"-------partial resting fill Latency------- \n\n";

    std::cout<<"-------partial incoming fill Latency-------";
    calculatePartialIncomingFillLatency();
    std::cout<<"-------partial incoming fill Latency------- \n\n";

    std::cout<<"-------multi level sweep Latency-------";
    calculateMultiLevelSweepLatency();
    std::cout<<"-------multi level sweep Latency------- \n\n";

    std::cout<<"-------deep book build Latency-------";
    calculateDeepBookBuildLatency();
    std::cout<<"-------deep book build Latency------- \n\n";

    std::cout<<"-------deep book cancel Latency-------";
    calculateDeepBookCancelLatency();
    std::cout<<"-------deep book cancel Latency------- \n\n";

    std::cout<<"-------deep book add Latency-------";
    calculateDeepBookAddLatency();
    std::cout<<"-------deep book add Latency------- \n\n";

    std::cout<<"-------deep book match Latency-------";
    calculateDeepBookMatchLatency();
    std::cout<<"-------deep book match Latency------- \n\n";

    return 0;
}
