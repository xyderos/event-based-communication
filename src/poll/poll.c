#include <fcntl.h>
#include <liburing.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "poll.h"

#define BUF_SIZE 512
#define FILE_NAME1 "/tmp/io_uring_sq_test.txt"
#define STR1 "gunga\n"
#define STR2 "ginga"

static int
start_fixed_buffer_ops(struct io_uring *ring)
{
	int fds[1];
	char buff1[BUF_SIZE];
	char buff2[BUF_SIZE];
	char buff3[BUF_SIZE];
	char buff4[BUF_SIZE];
	int ret;
	struct io_uring_sqe *sqe;
	struct io_uring_cqe *cqe;
	unsigned int str1_sz = strlen(STR1), str2_sz = strlen(STR2);

	fds[0] = open(FILE_NAME1, O_RDWR | O_TRUNC | O_CREAT, 0644);
	if (fds[0] < 0) {
		perror("open");
		return 1;
	}

	memset(buff1, 0, BUF_SIZE);
	memset(buff2, 0, BUF_SIZE);
	memset(buff3, 0, BUF_SIZE);
	memset(buff4, 0, BUF_SIZE);
	strncpy(buff1, STR1, str1_sz);
	strncpy(buff2, STR2, str2_sz);

	ret = io_uring_register_files(ring, fds, 1);
	if (ret) {
		return 1;
	}

	sqe = io_uring_get_sqe(ring);
	if (!sqe) {
		return 1;
	}
	io_uring_prep_write(sqe, 0, buff1, str1_sz, 0);
	io_uring_sqe_set_flags(sqe, IOSQE_FIXED_FILE);

	sqe = io_uring_get_sqe(ring);
	if (!sqe) {
		return 1;
	}
	io_uring_prep_write(sqe, 0, buff2, str2_sz, str1_sz);
	io_uring_sqe_set_flags(sqe, IOSQE_FIXED_FILE);

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
	io_uring_prep_read(sqe, 0, buff3, str1_sz, 0);
	io_uring_sqe_set_flags(sqe, IOSQE_FIXED_FILE);

	sqe = io_uring_get_sqe(ring);
	if (!sqe) {
		return 1;
	}
	io_uring_prep_read(sqe, 0, buff4, str2_sz, str1_sz);
	io_uring_sqe_set_flags(sqe, IOSQE_FIXED_FILE);

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
	printf("Contents read from file:\n");
	printf("%s%s", buff3, buff4);

	return 0;
}

int
poll(void)
{
	struct io_uring ring;
	struct io_uring_params params;
	int ret;

	if (geteuid()) {
		return 1;
	}

	memset(&params, 0, sizeof(params));
	params.flags |= IORING_SETUP_SQPOLL;
	params.sq_thread_idle = 2000;

	ret = io_uring_queue_init_params(8, &ring, &params);
	if (ret) {
		return 1;
	}
	start_fixed_buffer_ops(&ring);
	io_uring_queue_exit(&ring);
	return 0;
}
