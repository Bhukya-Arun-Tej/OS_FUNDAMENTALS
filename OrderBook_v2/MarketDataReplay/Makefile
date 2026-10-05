CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -Wpedantic

all: udp_publisher udp_receiver

udp_publisher: udp_publisher.cpp protocol.cpp protocol.hpp
	$(CXX) $(CXXFLAGS) udp_publisher.cpp protocol.cpp -o udp_publisher

udp_receiver: udp_receiver.cpp protocol.cpp protocol.hpp
	$(CXX) $(CXXFLAGS) udp_receiver.cpp protocol.cpp -o udp_receiver

clean: 
	rm -f udp_publisher udp_receiver
