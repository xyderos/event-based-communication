#include <liburing.h>
#include <stdio.h>
#include <unistd.h>

struct user_data {
	char buf[BUFSIZ];
	int socket_fd;
	int file_fd;
	int index;
	int io_op;
};

void prep_accept(struct io_uring *, int);

void prep_recv(struct io_uring *, int, int, int);

void prep_read(struct io_uring *, int);

void prep_send(struct io_uring *, int);

void lib_uring(unsigned int nof_requests);
