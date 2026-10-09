CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -Wpedantic

all: udp_receiver  market_data_server

.PHONY: all test clean

udp_receiver: udp_receiver.cpp protocol.cpp protocol.hpp tcp_recovery_client.hpp tcp_recovery_client.cpp tcphelper.cpp tcphelper.hpp packet_sequencer.cpp
	$(CXX) $(CXXFLAGS) udp_receiver.cpp protocol.cpp tcp_recovery_client.cpp tcphelper.cpp packet_sequencer.cpp -o udp_receiver

market_data_server: market_data_server.cpp feed_producer.cpp udp_publisher.cpp tcp_recovery_server.cpp feed_history.cpp protocol.cpp tcphelper.cpp
	$(CXX) $(CXXFLAGS) market_data_server.cpp feed_producer.cpp udp_publisher.cpp tcp_recovery_server.cpp feed_history.cpp protocol.cpp tcphelper.cpp -o market_data_server

test: protocol.cpp protocol.hpp protocol_test.cpp feed_history.cpp feed_history.hpp feed_history_test.cpp packet_sequencer.cpp packet_sequencer.hpp packet_sequencer_test.cpp
	$(CXX) $(CXXFLAGS) protocol.cpp protocol_test.cpp -o protocol_test
	$(CXX) $(CXXFLAGS) feed_history.cpp feed_history_test.cpp -o feed_history_test
	$(CXX) $(CXXFLAGS) protocol.cpp packet_sequencer.cpp packet_sequencer_test.cpp -o packet_sequencer_test
	./protocol_test
	./feed_history_test
	./packet_sequencer_test

clean: 
	rm -f udp_receiver protocol_test feed_history_test packet_sequencer_test market_data_server
