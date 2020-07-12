CC = gcc
CFLAGS += -Wall -g

all: shell.c
	$(CC) $(CFLAGS) $^ -o $(basename $^)

clean:
	rm -f shell
