#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

#include "bus.h"

void callback(void* _ctx, void* _msg) {
	int msg = *(int*) _msg;
	int ctx = *(int*) _ctx;
	printf("Callback for client %d received: 0x%x\n", ctx, msg);
}

int main() {
	Bus* bus;
	int msg0, msg1, ctx0, ctx1;

	ctx0 = 0;
	ctx1 = 1;
	msg0 = 0xdead;
	msg1 = 0xbeef;

	/* Create a new bus */
	if (!bus_new(&bus, 0))
		errx(EXIT_FAILURE, "error @ %s:%d: bus_new", __FILE__, __LINE__);

	/* Register clients with IDs 0 and 1 */
	if (!bus_register(bus, 0, &callback, &ctx0) || !bus_register(bus, 1, &callback, &ctx1)) {
		errx(EXIT_FAILURE, "error @ %s:%d: bus_register", __FILE__, __LINE__);
		bus_free(bus);
	}

	/* Send the message to all registered clients (broadcast=1) - clients 0 and 1 */
	if (!bus_send(bus, 0, &msg0, 1)) {
		errx(EXIT_FAILURE, "error @ %s:%d: bus_send", __FILE__, __LINE__);
		bus_free(bus);
	}

	/* Unregister client with ID = 1 */
	if (!bus_unregister(bus, 1)) {
		errx(EXIT_FAILURE, "error @ %s:%d: bus_unregister", __FILE__, __LINE__);
		bus_free(bus);	
	}

	/* Send the message to all registered clients (broadcast=1) - just client 0 */
	if (!bus_send(bus, 0, &msg1, 1)) {
		errx(EXIT_FAILURE, "error @ %s:%d: bus_send", __FILE__, __LINE__);
		bus_free(bus);
	}

	/* Delete the bus */
	bus_free(bus);
	
	return 0;
}