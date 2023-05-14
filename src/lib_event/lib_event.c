#define _GNU_SOURCE

#include "../helpers/connection.h"
#include "lib_event.h"

static int num_reqs = 0;

void
read_cb(struct bufferevent *bev, void *ctx)
{
	char buf[BUFSIZ] = "";
	struct evbuffer *output;
	int fd = 0;

	bufferevent_read(bev, buf, BUFSIZ);

	fd = open(buf, O_RDONLY | O_NONBLOCK);

	if (fd == -1) {
		handle_error("open");
	}

	output = bufferevent_get_output(bev);

	evbuffer_set_flags(output, EVBUFFER_FLAG_DRAINS_TO_FD);
	evbuffer_add_file(output, fd, 0, -1);
}

void
error_cb(struct bufferevent *bev, short error, void *ctx)
{
	struct event_base *base;

	if (error & BEV_EVENT_EOF) {
		if (--num_reqs == 0) {

			base = bufferevent_get_base(bev);

			event_base_loopexit(base, NULL);
		}
	} else if (error & BEV_EVENT_ERROR) {
		(void)fprintf(stderr, "error: %s\n", strerror(error));
	}
	bufferevent_free(bev);
}

void
do_accept(int sfd, short event, void *arg)
{
	struct event_base *base = arg;
	struct bufferevent *bev = NULL;
	int cfd = 0;

	cfd = accept4(sfd, NULL, NULL, SOCK_NONBLOCK);

	if (cfd == -1) {
		handle_error("accept4");
	}

	bev = bufferevent_socket_new(base, cfd, BEV_OPT_CLOSE_ON_FREE);

	bufferevent_setcb(bev, read_cb, NULL, error_cb, NULL);
	bufferevent_enable(bev, EV_READ | EV_WRITE);
}

void
lib_event(unsigned int nof_requests)
{
	struct event_base *base = NULL;
	struct event *listener_event = NULL;
	int sfd = 0;

	sfd = init_socket(1, 1);

	base = event_base_new();
	listener_event = event_new(base, sfd, EV_READ | EV_PERSIST, do_accept,
	    (void *)base);

	event_add(listener_event, NULL);
	event_base_dispatch(base);
	event_base_free(base);
}
