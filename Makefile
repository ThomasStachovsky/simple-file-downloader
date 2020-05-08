CC = gcc
CFLAGS = -std=c99 -Wall -Wextra

all: transport
transport: main.o transport.o error.o
	gcc $(CFLAGS) -o transport $^
main.o: transport.h error.h
transport.o: transport.h error.h
error.o: error.h

clean:
	rm -f *.o
distclean:
	rm -f *.o
	rm -f transport
