CC = gcc
CFLAGS = -Wall -Wextra -pthread -D_GNU_SOURCE
TARGETS = server client

all: $(TARGETS)

game_logic.o: game_logic.c game_common.h
	$(CC) $(CFLAGS) -c game_logic.c -o game_logic.o

server: server.c game_logic.o
	$(CC) $(CFLAGS) server.c game_logic.o -o server

client: client.c game_logic.o
	$(CC) $(CFLAGS) client.c game_logic.o -o client

clean:
	rm -f $(TARGETS) *.o

run_server: server
	./server

run_client: client
	./client

.PHONY: all clean run_server run_client