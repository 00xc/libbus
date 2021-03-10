#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <pthread.h>

#include "bus.h"

#define NUM_THREADS    4

/* Data passed to each thread */
typedef struct {
	Bus* bus;
	unsigned int id;
} ThreadData;

/* Function to be called by the bus for each new message */
void bus_callback(void* _ctx, void* _msg) {
	unsigned int ctx = *(unsigned int*) _ctx;
	unsigned int msg = *(unsigned int*) _msg;
	printf("Callback for thread %d received: %d\n", ctx, msg);
}

/* This funcion will be spawned `NUM_THREADS` times as a separate thread. */
void* thread_fn(void* _data) {
	ThreadData* data = (ThreadData*) _data;
	ClientId dest = (data->id + 1) % NUM_THREADS;

	/* Register our callback */
	if (!bus_register(data->bus, data->id, &bus_callback, &(data->id))) {
		fprintf(stderr, "Error registering client %d @ thread %d\n", data->id, data->id);
		return NULL;
	}
	printf("Registered callback from thread %d\n", data->id);

	/* Loop until the destination is registered from a separate thread */
	while (!bus_send(data->bus, dest, &(data->id), 0)) {
		;;
	}

	return NULL;
}

int main() {
	unsigned int i;
	Bus* bus;
	pthread_t threads[NUM_THREADS];
	ThreadData contexts[NUM_THREADS];

	srand(time(NULL));

	if (!bus_new(&bus, 0))
		errx(EXIT_FAILURE, "error @ %s:%d: bus_new", __FILE__, __LINE__);

	/* Launch threads, each with their own context containing a reference to the bus and their ID */
	for (i=0; i<NUM_THREADS; ++i) {

		contexts[i].bus = bus;
		contexts[i].id = i;

		if (pthread_create(&threads[i], NULL, thread_fn, &contexts[i])){
			fprintf(stderr, "Error creating thread %d\n", i);
		}
	}

	/* Wait until completion */
	for (i=0; i<NUM_THREADS; ++i) {
		if (pthread_join(threads[i], NULL) != 0) {
			fprintf(stderr, "Error joining thread %d\n", i);	
		}
	}

	bus_free(bus);
	
	return 0;
}