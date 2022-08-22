
CC = gcc
servermake:  client.c server.c
 
	$(CC) -o client client.c
	$(CC) -o server server.c
