#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffers.h"
#include "liburing.h"

#define BUF_SIZE 512
#define FILE_NAME1 "/tmp/io_uring_test.txt"
#define STR1 "who who,\n"
#define STR2 "what where ."

static int
start_fixed_buffer_ops(struct io_uring *ring)
{
	struct iovec iov[4];
	struct io_uring_sqe *sqe;
	struct io_uring_cqe *cqe;
	int ret;

	int fd = open(FILE_NAME1, O_RDWR | O_TRUNC | O_CREAT, 0644);
	if (fd < 0) {
		perror("open");
		return 1;
	}

	for (int i = 0; i < 4; i++) {
		iov[i].iov_base = malloc(BUF_SIZE);
		iov[i].iov_len = BUF_SIZE;
		memset(iov[i].iov_base, 0, BUF_SIZE);
	}

	ret = io_uring_register_buffers(ring, iov, 4);
	if (ret) {
		return 1;
	}

	sqe = io_uring_get_sqe(ring);
	if (!sqe) {
		return 1;
	}

	strncpy(iov[0].iov_base, STR1, strlen(STR1));
	strncpy(iov[1].iov_base, STR2, strlen(STR2));
	io_uring_prep_write_fixed(sqe, fd, iov[0].iov_base, strlen(STR1), 0, 0);

	sqe = io_uring_get_sqe(ring);
	if (!sqe) {
		return 1;
	}
	io_uring_prep_write_fixed(sqe, fd, iov[1].iov_base, strlen(STR2),
	    strlen(STR1), 1);

	io_uring_submit(ring);

	for (int i = 0; i < 2; i++) {
		ret = io_uring_wait_cqe(ring, &cqe);
		if (ret < 0) {
			return 1;
		}
		if (cqe->res < 0) {
			break;
		}
		printf("Result of the operation: %d\n", cqe->res);
		io_uring_cqe_seen(ring, cqe);
	}

	sqe = io_uring_get_sqe(ring);
	if (!sqe) {
		return 1;
	}

	io_uring_prep_read_fixed(sqe, fd, iov[2].iov_base, strlen(STR1), 0, 2);

	sqe = io_uring_get_sqe(ring);
	if (!sqe) {
		return 1;
	}

	io_uring_prep_read_fixed(sqe, fd, iov[3].iov_base, strlen(STR2),
	    strlen(STR1), 3);

	io_uring_submit(ring);
	for (int i = 0; i < 2; i++) {
		ret = io_uring_wait_cqe(ring, &cqe);
		if (ret < 0) {
			return 1;
		}
		/* Now that we have the CQE, let's process the data */
		if (cqe->res < 0) {
			break;
		}
		printf("Result of the operation: %d\n", cqe->res);
		io_uring_cqe_seen(ring, cqe);
	}

	return 0;
}

int
buffers(void)
{
	struct io_uring ring;

	int ret = io_uring_queue_init(8, &ring, 0);
	if (ret) {
		return 1;
	}
	start_fixed_buffer_ops(&ring);
	io_uring_queue_exit(&ring);
	return 0;
}
