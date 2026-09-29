book: Orderbook

Orderbook: Orderbook.cpp Orderbook.hpp
	g++ -std=c++17 Orderbook.cpp main.cpp -o Orderbook
run:
	./Orderbook
test:
	g++ -std=c++17 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wno-return-type -fsanitize=address,undefined -fno-omit-frame-pointer Orderbook_test.cpp -o Orderbook_test
	./Orderbook_test
clean:
	rm -rf Orderbook Orderbook_test
