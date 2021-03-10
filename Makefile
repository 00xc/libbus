CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -O3 -std=c99 -fPIC

OBJECTS = bin/bus.o
STATIC_LIB = libbus.a

bin/%.o: src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

all: static thread_program simple_program

static: $(OBJECTS)
	ar rcs $(STATIC_LIB) $?

simple_program: static
	$(CC) $(CFLAGS) examples/simple_program.c libbus.a -Isrc/ -latomic -o $@

thread_program: static
	$(CC) $(CFLAGS) examples/thread_program.c libbus.a -Isrc/ -latomic -pthread -o $@

clean:
	rm -f $(OBJECTS)
	rm -f $(STATIC_LIB)
	rm -f simple_program thread_program