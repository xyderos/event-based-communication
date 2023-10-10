#include <sys/ioctl.h>
#include <sys/stat.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <liburing.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cp.h"

#define QD 2
#define BS (16u * 1024u)

static int infd, outfd;

struct io_data {
	int read;
	off_t first_offset, offset;
	size_t first_len;
	struct iovec iov;
};

static int
setup_context(unsigned entries, struct io_uring *ring)
{
	int ret;

	ret = io_uring_queue_init(entries, ring, 0);
	if (ret < 0) {
		return -1;
	}

	return 0;
}

static int
get_file_size(int fd, off_t *size)
{
	struct stat st;

	if (fstat(fd, &st) < 0)
		return -1;
	if (S_ISREG(st.st_mode)) {
		*size = st.st_size;
		return 0;
	} else if (S_ISBLK(st.st_mode)) {
		unsigned long long bytes;

		if (ioctl(fd, BLKGETSIZE64, &bytes) != 0)
			return -1;

		*size = (long)bytes;
		return 0;
	}
	return -1;
}

static void
queue_prepped(struct io_uring *ring, struct io_data *data)
{
	struct io_uring_sqe *sqe;

	sqe = io_uring_get_sqe(ring);
	assert(sqe);

	if (data->read)
		io_uring_prep_readv(sqe, infd, &data->iov, 1,
		    (unsigned)data->offset);
	else
		io_uring_prep_writev(sqe, outfd, &data->iov, 1,
		    (unsigned)data->offset);

	io_uring_sqe_set_data(sqe, data);
}

static int
queue_read(struct io_uring *ring, off_t size, off_t offset)
{
	struct io_uring_sqe *sqe;
	struct io_data *data;

	data = malloc((unsigned)size + sizeof(*data));
	if (!data)
		return 1;

	sqe = io_uring_get_sqe(ring);
	if (!sqe) {
		free(data);
		return 1;
	}

	data->read = 1;
	data->offset = data->first_offset = offset;

	data->iov.iov_base = data + 1;
	data->iov.iov_len = (unsigned)size;
	data->first_len = (unsigned)size;

	io_uring_prep_readv(sqe, infd, &data->iov, 1, (unsigned)offset);
	io_uring_sqe_set_data(sqe, data);
	return 0;
}

static void
queue_write(struct io_uring *ring, struct io_data *data)
{
	data->read = 0;
	data->offset = data->first_offset;

	data->iov.iov_base = data + 1;
	data->iov.iov_len = data->first_len;

	queue_prepped(ring, data);
	io_uring_submit(ring);
}

static int
copy_file(struct io_uring *ring, off_t insize)
{
	unsigned long reads, writes;
	struct io_uring_cqe *cqe;
	off_t write_left, offset;
	int ret;

	write_left = insize;
	writes = reads = offset = 0;

	while (insize || write_left) {
		int had_reads, got_comp;

		had_reads = (int)reads;
		while (insize) {
			off_t this_size = insize;

			if (reads + writes >= QD)
				break;
			if (this_size > (long)BS)
				this_size = (long)BS;
			else if (!this_size)
				break;

			if (queue_read(ring, this_size, offset))
				break;

			insize -= this_size;
			offset += this_size;
			reads++;
		}

		if ((unsigned long)had_reads != reads) {
			ret = io_uring_submit(ring);
			if (ret < 0) {
				break;
			}
		}

		got_comp = 0;
		while (write_left) {
			struct io_data *data;

			if (!got_comp) {
				ret = io_uring_wait_cqe(ring, &cqe);
				got_comp = 1;
			} else {
				ret = io_uring_peek_cqe(ring, &cqe);
				if (ret == -EAGAIN) {
					cqe = NULL;
					ret = 0;
				}
			}
			if (ret < 0) {
				return 1;
			}
			if (!cqe)
				break;

			data = io_uring_cqe_get_data(cqe);
			if (cqe->res < 0) {
				if (cqe->res == -EAGAIN) {
					queue_prepped(ring, data);
					io_uring_cqe_seen(ring, cqe);
					continue;
				}

				return 1;
			} else if ((unsigned long)cqe->res !=
			    data->iov.iov_len) {

				data->iov.iov_base += cqe->res;
				data->iov.iov_len -= (unsigned)cqe->res;
				queue_prepped(ring, data);
				io_uring_cqe_seen(ring, cqe);
				continue;
			}

			if (data->read) {
				queue_write(ring, data);
				write_left -= (long)data->first_len;
				reads--;
				writes++;
			} else {
				free(data);
				writes--;
			}
			io_uring_cqe_seen(ring, cqe);
		}
	}

	return 0;
}

int
cp(char **argv)
{
	struct io_uring ring;
	off_t insize;
	int ret;

	infd = open(argv[1], O_RDONLY);
	if (infd < 0) {
		perror("open infile");
		return 1;
	}

	outfd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (outfd < 0) {
		perror("open outfile");
		return 1;
	}

	if (setup_context(QD, &ring))
		return 1;

	if (get_file_size(infd, &insize))
		return 1;

	ret = copy_file(&ring, insize);

	close(infd);
	close(outfd);
	io_uring_queue_exit(&ring);
	return ret;
}
