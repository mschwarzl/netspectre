all: client server

client: client.c
	gcc client.c -o client

server: server.c
	gcc server.c -o server -pthread -mavx2
	
clean:
	rm -f server client
