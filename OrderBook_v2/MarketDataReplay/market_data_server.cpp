#include "feed_history.hpp"
#include "feed_producer.hpp"
#include "tcp_recovery_server.hpp"
#include <functional>
#include <thread>

int main(){
    FeedHistory history{1024};

    std::thread tcpRecoveryThread(runTcpRecoveryServer,std::cref(history));
    const int producer = runFeedProducer(history);
    tcpRecoveryThread.join();
    return producer;
}