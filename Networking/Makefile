all: clean server client
server:
	g++ -std=c++20 simple_tcp_server.cpp -o server
client:
	g++ -std=c++20 simple_tcp_client.cpp -o client
clean:
	rm -f server client
