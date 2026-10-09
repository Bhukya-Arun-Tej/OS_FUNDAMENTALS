CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -Wpedantic

book: Orderbook

Orderbook: Orderbook.cpp Orderbook.hpp Trade.cpp Trade.hpp TradeInfo.cpp TradeInfo.hpp OrderLocation.cpp OrderLocation.hpp Order.cpp Order.hpp
	$(CXX) $(CXXFLAGS) Orderbook.cpp Trade.cpp TradeInfo.cpp OrderLocation.cpp Order.cpp main.cpp -o Orderbook
run:
	./Orderbook
test:
	#g++ -std=c++17 -Wconversion -Wshadow -Wno-return-type -fsanitize=address,undefined -fno-omit-frame-pointer Orderbook_test.cpp -o Orderbook_test
	
	$(CXX) $(CXXFLAGS) Orderbook.cpp Trade.cpp TradeInfo.cpp OrderLocation.cpp Order.cpp Orderbook_test.cpp -o Orderbook_test
	./Orderbook_test
bench:
	$(CXX) $(CXXFLAGS) -O3 -DNDEBUG -march=native Orderbook.cpp Trade.cpp TradeInfo.cpp OrderLocation.cpp Order.cpp Orderbook_benchmark.cpp -o Orderbook_benchmark
	./Orderbook_benchmark
clean:
	rm -rf Orderbook Orderbook_test Orderbook_benchmark
