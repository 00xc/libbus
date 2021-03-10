CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -O3 -std=c99 -fPIC

OBJECTS = bin/bus.o
STATIC_LIB = libbus.a

bin/%.o: src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

all: static main

static: $(OBJECTS)
	ar rcs $(STATIC_LIB) $?

main: static
	gcc $(CFLAGS) src/main.c libbus.a -Isrc/ -latomic -o $@

clean:
	rm -f $(OBJECTS)
	rm -f $(STATIC_LIB)
	rm -f main