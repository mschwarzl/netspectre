all: client server

client: client.c
	gcc client.c -o client -O0

server: server.c
	gcc server.c -o server -pthread -mavx2 -O0
	
clean:
	rm -f server client
