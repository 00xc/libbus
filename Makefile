CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Wstrict-aliasing -O3 -std=c99 -fPIC
LDFLAGS = -Isrc/ -latomic

OBJECTS = bin/bus.o
STATIC_LIB = libbus.a

.PHONY: all static clean

bin/%.o: src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

all: static simple_program thread_program

static: $(STATIC_LIB)

$(STATIC_LIB): $(OBJECTS)
	ar rcs $(STATIC_LIB) $?

simple_program: examples/simple_program.c static
	$(CC) $(CFLAGS) $< $(STATIC_LIB) $(LDFLAGS) -o $@

thread_program: examples/thread_program.c static
	$(CC) $(CFLAGS) $< $(STATIC_LIB) $(LDFLAGS) -pthread -o $@

clean:
	rm -f $(OBJECTS)
	rm -f $(STATIC_LIB)
	rm -f simple_program thread_program