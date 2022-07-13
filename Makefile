CFLAGS= -g -Wall -pedantic

all: client server

client: client.c
	gcc -o client $(CFLAGS) client.c

server: server.c
	gcc -o server $(CFLAGS) server.c
