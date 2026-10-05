CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -Wpedantic

all: udp_publisher udp_receiver

udp_publisher: udp_publisher.cpp
	$(CXX) $(CXXFLAGS) udp_publisher.cpp -o udp_publisher

udp_receiver: udp_receiver.cpp
	$(CXX) $(CXXFLAGS) udp_receiver.cpp -o udp_receiver

clean: 
	rm -f udp_publisher udp_receiver