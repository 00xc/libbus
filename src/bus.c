#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "bus.h"

#define CAS(dst, expected, value) __atomic_compare_exchange(dst, expected, value, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)

typedef struct {
	unsigned int registered;
	unsigned int refcount;
	ClientCallback callback;
	void* ctx;
} BusClient;

struct _Bus {
	BusClient* clients;
	const unsigned int num_clients;
};

/*
 * Attempts to call a client's callback to send a message. Might fail if such client gets
 * unregistered while attempting to send the message.
 */
static int attempt_client_callback(BusClient* client, void* msg) {
	BusClient local_client, new_client;

	/* Load the client we are attempting to communicate with */
	__atomic_load(client, &local_client, __ATOMIC_SEQ_CST);

	/* Loop until we update the refcount or the client becomes unregistered */
	while (local_client.registered) {

		/* Our desired refcount is the current one + 1 */
		new_client = local_client;
		++(new_client.refcount);

		/*
		 * If CAS succeeds, the client had the expected refcount, and we updated it successfully.
		 * If CAS fails, the client was updated recently. The actual value is copied to `local_client`.
		 */
		if (CAS(client, &local_client, &new_client)) {

			/* Send a message and decrease the refcount back */
			local_client.callback(local_client.ctx, msg);
			__atomic_fetch_sub(&(client->refcount), 1, __ATOMIC_SEQ_CST);

			return 1;
		}
	}

	/* Client was not registered, or got unregistered while we attempted to send a message */
	return 0;
}

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

	BusClient null_client = {0};
	BusClient new_client = { .registered = 1, .callback = callback, .ctx = ctx, .refcount = 0 };

	return (int) CAS(&(bus->clients[id]), &null_client, &new_client);
}

int bus_send(Bus* bus, ClientId id, void* msg, int broadcast) {

	if (broadcast) {

		for (id = 0; id < bus->num_clients; ++id) {
			attempt_client_callback(&(bus->clients[id]), msg);
		}

	} else {

		if (id >= bus->num_clients)
			return 0;
		return attempt_client_callback(&(bus->clients[id]), msg);
	}

	return 1;
}

int bus_unregister(Bus* bus, ClientId id) {
	BusClient local_client;
	BusClient null_client = {0};

	if (id >= bus->num_clients)
		return 0;

	/* Load the client we are attempting to unregister */
	__atomic_load(&(bus->clients[id]), &local_client, __ATOMIC_SEQ_CST);

	/* It was already unregistered */
	if (local_client.registered == 0)
		return 0;

	do {

		/* Our desired refcount is 0 */
		local_client.refcount = 0;

		/*
		 * If CAS succeeds, the client had refcount=0 and got unregistered
		 * If CAS does not succeed, the value of the client gets copied into `local_client`.
		 */
		if (CAS(&(bus->clients[id]), &local_client, &null_client)) {
			return 1;
		}

	} while (local_client.registered);

	/* Someone else unregistered this client */
	return 1;
}

void bus_free(Bus* bus) {

	if (bus) {
		if (bus->clients) {
			free(bus->clients);
			bus->clients = NULL;
		}
		free(bus);
		bus = NULL;
	}
}
