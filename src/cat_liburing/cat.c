#include <sys/ioctl.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <liburing.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cat.h"

#define QUEUE_DEPTH 1
#define BLOCK_SZ 1024

struct file_info {
	off_t file_sz;
	struct iovec iovecs[];
};

static off_t
get_file_size(int fd)
{
	struct stat st;

	if (fstat(fd, &st) < 0) {
		perror("fstat");
		return -1;
	}
	if (S_ISBLK(st.st_mode)) {
		unsigned long long bytes;
		if (ioctl(fd, BLKGETSIZE64, &bytes) != 0) {
			perror("ioctl");
			return -1;
		}
		return (off_t)bytes;
	} else if (S_ISREG(st.st_mode))
		return st.st_size;

	return -1;
}

static void
output_to_console(char *buf, int len)
{
	while (len--) {
		(void)fputc(*buf++, stdout);
	}
}

static int
get_completion_and_print(struct io_uring *ring)
{
	struct io_uring_cqe *cqe;
	struct file_info *fi;

	int ret = io_uring_wait_cqe(ring, &cqe), blocks;
	if (ret < 0) {
		perror("io_uring_wait_cqe");
		return 1;
	}

	if (cqe->res < 0) {
		(void)fprintf(stderr, "Async readv failed.\n");
		return 1;
	}

	fi = io_uring_cqe_get_data(cqe);
	blocks = (int)fi->file_sz / BLOCK_SZ;

	if (fi->file_sz % BLOCK_SZ)
		blocks++;

	for (int i = 0; i < blocks; i++)
		output_to_console(fi->iovecs[i].iov_base,
		    (int)fi->iovecs[i].iov_len);

	io_uring_cqe_seen(ring, cqe);
	return 0;
}

static int
submit_read_request(char *file_path, struct io_uring *ring)
{
	off_t file_sz, bytes_remaining;
	struct file_info *fi;
	struct io_uring_sqe *sqe;
	void *buf;
	int file_fd = open(file_path, O_RDONLY), current_block = 0, blocks;
	if (file_fd < 0) {
		perror("open");
		return 1;
	}

	file_sz = get_file_size(file_fd);
	bytes_remaining = file_sz;
	blocks = (int)file_sz / BLOCK_SZ;

	if (file_sz % BLOCK_SZ)
		blocks++;
	fi = malloc(
	    sizeof(*fi) + (sizeof(struct iovec) * (unsigned long)blocks));

	while (bytes_remaining) {
		off_t bytes_to_read = bytes_remaining;
		if (bytes_to_read > BLOCK_SZ)
			bytes_to_read = BLOCK_SZ;

		fi->iovecs[current_block].iov_len = (unsigned long)
		    bytes_to_read;

		if (posix_memalign(&buf, BLOCK_SZ, BLOCK_SZ)) {
			perror("posix_memalign");
			return 1;
		}
		fi->iovecs[current_block].iov_base = buf;

		current_block++;
		bytes_remaining -= bytes_to_read;
	}
	fi->file_sz = file_sz;

	sqe = io_uring_get_sqe(ring);
	io_uring_prep_readv(sqe, file_fd, fi->iovecs, (unsigned)blocks, 0);
	io_uring_sqe_set_data(sqe, fi);
	io_uring_submit(ring);

	return 0;
}

int
cat(int argc, char **argv)
{
	struct io_uring ring;

	io_uring_queue_init(QUEUE_DEPTH, &ring, 0);

	for (int i = 1; i < argc; i++) {
		int ret = submit_read_request(argv[i], &ring);
		if (ret) {
			return 1;
		}
		get_completion_and_print(&ring);
	}

	io_uring_queue_exit(&ring);
	return 0;
}
