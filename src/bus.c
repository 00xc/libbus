#include <string.h>
#include <stdlib.h>

#include "bus.h"

typedef struct {
	ClientId id;
	ClientCallback callback;
	void* ctx;
} BusClient;

struct _Bus {
	BusClient* clients;
	const unsigned int num_clients;
};


int bus_new(Bus** bus, unsigned int num_clients) {
	Bus* b;

	if (num_clients > BUS_MAX_CLIENTS)
		return 0;

	if ( (b = malloc(sizeof(Bus))) == NULL )
		return 0;

	/* Initialize bus struct */
	*(unsigned int*)&b->num_clients = num_clients == 0 ? BUS_DEFAULT_CLIENTS : num_clients;
	if ( (b->clients = calloc(b->num_clients, sizeof(BusClient))) == NULL ) {
		free(b);
		return 0;
	}

	*bus = b;

	return 1;
}

int bus_register(Bus* bus, ClientId id, ClientCallback callback, void* ctx) {

	if (id >= bus->num_clients)
		return 0;

	BusClient new_client = { .id=id, callback = callback, .ctx = ctx };
	BusClient null_client = {0};

	return (int) __atomic_compare_exchange(&(bus->clients[id]), &null_client, &new_client, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

int bus_send(Bus* bus, ClientId id, void* msg, int broadcast) {
	BusClient client;

	if (broadcast) {

		for (unsigned int i = 0; i < bus->num_clients; ++i) {
			__atomic_load(&(bus->clients[i]), &client, __ATOMIC_SEQ_CST);
			if (client.callback)
				client.callback(client.ctx, msg);
		}		

	} else {

		if (id >= bus->num_clients)
			return 0;

		__atomic_load(&(bus->clients[id]), &client, __ATOMIC_SEQ_CST);
		if (client.callback == NULL)
			return 0;
		client.callback(client.ctx, msg);

	}

	return 1;
}

int bus_unregister(Bus* bus, ClientId id) {
	BusClient b = {0};

	if (id >= bus->num_clients)
		return 0;

	__atomic_store(&(bus->clients[id]), &b, __ATOMIC_SEQ_CST);
	return 1;
}

void bus_free(Bus* bus) {

	if (bus) {
		if (bus->clients)
			free(bus->clients);	
		free(bus);
	}
	
	return;
}
