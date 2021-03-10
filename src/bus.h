#ifndef _COMBUS
#define _COMBUS

#include <limits.h>

#define BUS_DEFAULT_CLIENTS    128
#define BUS_MAX_CLIENTS        UINT_MAX

typedef struct _Bus Bus;
typedef unsigned int ClientId;
typedef void (*ClientCallback)(void* ctx, void* msg);

/*
 * Allocates a new bus. If `num_clients` is non-zero, it allocates space for said number of
 * clients; otherwise, it uses `BUS_DEFAULT_CLIENTS`. `num_clients` cannot be greater than
 * `BUS_MAX_CLIENTS`.
 * Returns 1 on success, 0 on failure.
 */
int bus_new(Bus** bus, unsigned int num_clients) __attribute__((warn_unused_result));

/* 
 * Registers a new client with the specified ID. The ID must satisfy 0 <= ID < `num_clients` and
 * not be in use; otherwise the function fails.
 * Whenever a message is sent to this client, `callback` will be called.
 * The first argument for `callback` will the the user supplied context, `ctx` (can be ommitted
 * by passing NULL).
 * The second argument for `callback` will be the received message.
 * Returns 1 on success, 0 on failure.
 */
int bus_register(Bus* bus, ClientId id, ClientCallback callback, void* ctx) __attribute__((warn_unused_result));

/*
 * If broadcast is set to 0, it sends a message to the client with the specified ID.
 * If broadcast is set to 1, the message is sent to every registered client, and the supplied ID is
 * ignored.
 * Returns 1 on success, 0 on failure.
 */
int bus_send(Bus* bus, ClientId id, void* msg, int broadcast) __attribute__((warn_unused_result));

/*
 * Unregisters the client with the specified ID.
 * Returns 1 on success, 0 on failure.
 */
int bus_unregister(Bus* bus, ClientId id) __attribute__ ((warn_unused_result));

/*
 * Frees the bus object.
 */
void bus_free(Bus* bus);

#endif
