#ifndef _COMBUS
#define _COMBUS

#include <limits.h>

/* Avoid using GCC attributes if we're not compiling with GCC */
#ifdef __GNUC__
#	define ATTRIBUTE1(x)    __attribute__((x))
#	define ATTRIBUTE2(x, y) __attribute__((x, y))
#else
#	define ATTRIBUTE1(x)
#	define ATTRIBUTE2(x,y)
#endif

#define BUS_DEFAULT_CLIENTS    128
#define BUS_MAX_CLIENTS        UINT_MAX
#define BUS_NOBLOCK            1 << 0
#define BUS_BROADCAST          1 << 1

typedef struct _Bus Bus;
typedef unsigned int ClientId;
typedef void (*ClientCallback)(void* ctx, void* msg);

/*
 * Allocates a new bus. If `num_clients` is non-zero, it allocates space for said number of
 * clients; otherwise, it uses `BUS_DEFAULT_CLIENTS`.
 * `num_clients` cannot be greater than `BUS_MAX_CLIENTS`.
 * Returns 1 on success, 0 on failure.
 */
int ATTRIBUTE1(warn_unused_result) bus_new(Bus** bus, unsigned int num_clients);

/* 
 * Registers a new client with the specified ID. Whenever a message is sent to this client,
 * `callback` will be called.
 * The ID must satisfy 0 <= ID < `num_clients` and not be in use; otherwise the function fails.
 * The first argument for `callback` is the the user-supplied context, `ctx`. It can be ommitted
 * by passing NULL.
 * The second argument for `callback` will be the received message.
 * Returns 1 on success, 0 on failure.
 */
int ATTRIBUTE2(warn_unused_result, nonnull(1)) bus_register(Bus* bus, ClientId id, ClientCallback callback, void* ctx);

/*
 * Sends a message to the client with the specified ID.
 * Available flags:
 * `BUS_NOBLOCK`: if set, it will attempt once to send the message, failing if the call
 * would block or the client becomes unregistered. Otherwise, it retries until the message is
 * sent (success), or the client becomes unregistered (failure).
 * `BUS_BROADCAST`: if set, the message will be sent to every registered client, and
 * the `id` parameter will be ignored. Always succeeds.
 * Returns 1 on success, 0 on failure.
 */
int ATTRIBUTE2(warn_unused_result, nonnull(1)) bus_send(Bus* bus, ClientId id, void* msg, int flags);

/*
 * Unregisters the client with the specified ID. No messages can be sent to the specified client
 * once the function returns a success.
 * Succeeds if the client is unregistered successfully or if it was unregistered from a separate
 * thread.
 * Fails if the client was already unregistered.
 * Returns 1 on success, 0 on failure.
 */
int ATTRIBUTE2(warn_unused_result, nonnull(1)) bus_unregister(Bus* bus, ClientId id);

/*
 * Frees the bus object.
 */
void bus_free(Bus* bus);

#endif
