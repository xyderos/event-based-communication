#include <sys/eventfd.h>

#include <fcntl.h>
#include <liburing.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "event.h"

#define BUFF_SZ 512

static char buff[BUFF_SZ + 1];
static struct io_uring ring;

static __attribute__((noreturn)) void
error_exit(char *message)
{
	perror(message);
	exit(EXIT_FAILURE);
}

static void *
listener_thread(void *data)
{
	struct io_uring_cqe *cqe;
	unsigned long efd = (unsigned long)data;
	eventfd_t v;

	int ret = eventfd_read((int)efd, &v);
	if (ret < 0)
		error_exit("eventfd_read");

	ret = io_uring_wait_cqe(&ring, &cqe);
	if (ret < 0) {
		return NULL;
	}
	if (cqe->res < 0) {
	}

	io_uring_cqe_seen(&ring, cqe);

	return NULL;
}

static int
setup_io_uring(int efd)
{
	int ret = io_uring_queue_init(8, &ring, 0);
	if (ret) {
		return 1;
	}
	io_uring_register_eventfd(&ring, efd);
	return 0;
}

static int
read_file_with_io_uring(void)
{
	struct io_uring_sqe *sqe;
	int fd;

	sqe = io_uring_get_sqe(&ring);
	if (!sqe) {
		return 1;
	}

	fd = open("/etc/passwd", O_RDONLY);
	io_uring_prep_read(sqe, fd, buff, BUFF_SZ, 0);
	io_uring_submit(&ring);

	return 0;
}

int
register_event(void)
{
	pthread_t t;
	int efd;

	efd = eventfd(0, 0);
	pthread_create(&t, NULL, listener_thread, &efd);
	sleep(2);
	setup_io_uring(efd);
	read_file_with_io_uring();
	pthread_join(t, NULL);
	io_uring_queue_exit(&ring);
	return EXIT_SUCCESS;
}
