CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -Wpedantic

all: udp_publisher udp_receiver tcp_recovery_server tcp_recovery_client

udp_publisher: udp_publisher.cpp protocol.cpp protocol.hpp
	$(CXX) $(CXXFLAGS) udp_publisher.cpp protocol.cpp -o udp_publisher

udp_receiver: udp_receiver.cpp protocol.cpp protocol.hpp
	$(CXX) $(CXXFLAGS) udp_receiver.cpp protocol.cpp -o udp_receiver

tcp_recovery_server: tcp_recovery_server.cpp protocol.cpp protocol.hpp
	$(CXX) $(CXXFLAGS) tcp_recovery_server.cpp protocol.cpp -o tcp_recovery_server

tcp_recovery_client: tcp_recovery_client.cpp protocol.cpp protocol.hpp
	$(CXX) $(CXXFLAGS) tcp_recovery_client.cpp protocol.cpp -o tcp_recovery_client

test: protocol.cpp protocol.hpp protocol_test.cpp
	$(CXX) $(CXXFLAGS) protocol.cpp protocol_test.cpp -o protocol_test
	./protocol_test

clean: 
	rm -f udp_publisher udp_receiver protocol_test tcp_recovery_client tcp_recovery_server
