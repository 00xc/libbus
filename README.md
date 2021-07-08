# libbus #
A radically lightweight and simple concurrent message bus library.

## What does it do? ##
`libbus` provides a shared bus for message passing. Clients can register callbacks to receive messages from the bus. Any user can send a message to all registered clients (broadcast) or to specific ones. This library makes use of GCC's atomic builtins to ensure lock-free synchronization in a mulithreaded environment. More details can be found in [this blog post](https://scavengersecurity.com/posts/libbus/).

## Compiling ##
Simply use `make static` to compile as a static library. You can use `libbus` from your program by linking it with the library:

`gcc <your_program.c> libbus.a -I<path_to_libbus>/src/ -latomic`.

## Usage ##

### Use cases ###
An example of an use case would be an environment with multiple threads that perform jobs from a queue. With `libbus`, each thread can register a callback that pushes the received message into that thread's queue. In this case, one would set the `ctx` parameter of `bus_register` to point to that thread's queue. This way, threads can send messages to each other, using `bus_send`, just by having a reference to the shared bus.

### Examples ###
Two example programs are provided:
* [simple_program.c](examples/simple_program.c) registers two clients, sends a broadcast message, unregisters one of the clients and then sends another broadcast message. Each message is printed to standard output. It can me compiled with `make simple_program`.
* [thread_program.c](examples/thread_program.c) launches several threads, which send messages to each other and again get printed to standard out. It can me compiled with `make thread_program`.

The main usage flow consists on creating a new bus with `bus_new`, registering callbacks with `bus_register`, sending messages with `bus_send`, unregistering callbacks with `bus_unregister` and deleting the bus with `bus_free`.

### API ###
As mentioned, the library is radically simple; there are only 5 functions. The following documentation can be found in [bus.h](src/bus.h):

```C
#define BUS_DEFAULT_CLIENTS    128
#define BUS_MAX_CLIENTS        UINT_MAX

typedef unsigned int ClientId;
typedef void (*ClientCallback)(void* ctx, void* msg);

/*
 * Allocates a new bus. If `num_clients` is non-zero, it allocates space for said number of
 * clients; otherwise, it uses `BUS_DEFAULT_CLIENTS`.
 * `num_clients` cannot be greater than `BUS_MAX_CLIENTS`.
 * Returns 1 on success, 0 on failure.
 */
int bus_new(Bus** bus, unsigned int num_clients);

/* 
 * Registers a new client with the specified ID. Whenever a message is sent to this client,
 * `callback` will be called.
 * The ID must satisfy 0 <= ID < `num_clients` and not be in use; otherwise the function fails.
 * The first argument for `callback` is the the user-supplied context, `ctx`. It can be ommitted
 * by passing NULL.
 * The second argument for `callback` will be the received message.
 * Returns 1 on success, 0 on failure.
 */
int bus_register(Bus* bus, ClientId id, ClientCallback callback, void* ctx);

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
int bus_send(Bus* bus, ClientId id, void* msg, int flags);

/*
 * Unregisters the client with the specified ID. No messages can be sent to the specified client
 * once the function returns a success.
 * Succeeds if the client is unregistered successfully or if it was unregistered from a separate
 * thread.
 * Fails if the client was already unregistered.
 * Returns 1 on success, 0 on failure.
 */
int bus_unregister(Bus* bus, ClientId id);

/*
 * Frees the bus object.
 */
void bus_free(Bus* bus);
```
