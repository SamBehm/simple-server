CFLAGS= -g -Wall -pedantic

client: client.c
	gcc -o client $(CFLAGS) client.c
