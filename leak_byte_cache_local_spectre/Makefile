all: socket_server.c
	gcc socket_server.c -O0 -o server -lpthread -Wall
	gcc socket_client.c -O0 -o client -Wall -lm
	
clean:
	rm -f server client
