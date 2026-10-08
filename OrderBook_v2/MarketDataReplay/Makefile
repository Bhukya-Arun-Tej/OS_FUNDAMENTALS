CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -Wpedantic

all: udp_receiver check-components

.PHONY: all check-components test clean

check-components: udp_publisher.cpp udp_publisher.hpp tcp_recovery_server.cpp tcp_recovery_server.hpp feed_history.hpp tcphelper.hpp
	$(CXX) $(CXXFLAGS) -fsyntax-only udp_publisher.cpp
	$(CXX) $(CXXFLAGS) -fsyntax-only tcp_recovery_server.cpp

udp_receiver: udp_receiver.cpp protocol.cpp protocol.hpp tcp_recovery_client.hpp tcp_recovery_client.cpp tcphelper.cpp tcphelper.hpp
	$(CXX) $(CXXFLAGS) udp_receiver.cpp protocol.cpp tcp_recovery_client.cpp tcphelper.cpp -o udp_receiver

test: protocol.cpp protocol.hpp protocol_test.cpp feed_history.cpp feed_history.hpp feed_history_test.cpp
	$(CXX) $(CXXFLAGS) protocol.cpp protocol_test.cpp -o protocol_test
	$(CXX) $(CXXFLAGS) feed_history.cpp feed_history_test.cpp -o feed_history_test
	./protocol_test
	./feed_history_test

clean: 
	rm -f udp_publisher udp_receiver protocol_test feed_history_test tcp_recovery_server tcp_recovery_server.o
