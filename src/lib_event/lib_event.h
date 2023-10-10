#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <unistd.h>

#include "../helpers/connection.h"

void read_cb(struct bufferevent *, void *);

void error_cb(struct bufferevent *, short, void *);

void do_accept(int, short, void *);

void lib_event(void);
