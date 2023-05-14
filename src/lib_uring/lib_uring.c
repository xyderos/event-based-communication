#include "../helpers/connection.h"
#include "lib_uring.h"

static struct user_data data_arr[LISTEN_BACKLOG];

static unsigned int number_of_accepts = 0, number_of_requests = 0;

void
prep_accept(struct io_uring *ring, int sfd)
{
	struct io_uring_sqe *sqe = io_uring_get_sqe(ring);

	if (!sqe) {
		handle_error("io_uring_get_sqe");
	}

	io_uring_prep_accept(sqe, sfd, NULL, NULL, 0);

	data_arr[--number_of_accepts].io_op = IORING_OP_ACCEPT;
	data_arr[number_of_accepts].index = (int)number_of_accepts;

	io_uring_sqe_set_data(sqe, &data_arr[number_of_accepts]);

	if (io_uring_submit(ring) < 0) {
		handle_error("io_uring_submit");
	}
}

void
prep_recv(struct io_uring *ring, int sfd, int cfd, int index)
{
	struct io_uring_sqe *sqe = io_uring_get_sqe(ring);

	if (!sqe) {
		handle_error("io_uring_get_sqe");
	}

	data_arr[index].io_op = IORING_OP_RECV;
	data_arr[index].socket_fd = cfd;

	memset(data_arr[index].buf, 0, BUFSIZ);

	io_uring_prep_recv(sqe, cfd, data_arr[index].buf, BUFSIZ, 0);
	io_uring_sqe_set_data(sqe, &data_arr[index]);

	if (number_of_accepts > 0) {
		prep_accept(ring, sfd);
	}

	else if (io_uring_submit(ring) < 0) {
		handle_error("io_uring_submit");
	}
}

void
prep_read(struct io_uring *ring, int index)
{
	struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
	int file_fd = 0;

	if (!sqe) {
		handle_error("io_uring_get_sqe");
	}

	file_fd = open(data_arr[index].buf, O_RDONLY);

	if (file_fd == -1) {
		(void)fprintf(stderr, "buf: %s\n", data_arr[index].buf);
		handle_error("open");
	}

	data_arr[index].io_op = IORING_OP_READ;
	data_arr[index].file_fd = file_fd;

	memset(data_arr[index].buf, 0, BUFSIZ);

	io_uring_prep_read(sqe, file_fd, data_arr[index].buf, BUFSIZ, 0);
	io_uring_sqe_set_data(sqe, &data_arr[index]);

	if (io_uring_submit(ring) < 0) {
		handle_error("io_uring_submit");
	}
}

void
prep_send(struct io_uring *ring, int index)
{
	struct io_uring_sqe *sqe = io_uring_get_sqe(ring);

	if (!sqe) {
		handle_error("io_uring_get_sqe");
	}

	close(data_arr[index].file_fd);

	data_arr[index].io_op = IORING_OP_SEND;

	io_uring_prep_send(sqe, data_arr[index].socket_fd, data_arr[index].buf,
	    BUFSIZ, 0);
	io_uring_sqe_set_data(sqe, &data_arr[index]);

	if (io_uring_submit(ring) < 0) {
		handle_error("io_uring_submit");
	}
}

void
lib_uring(unsigned int nof_requests)
{
	int sfd = 0;
	struct io_uring ring;
	struct io_uring_cqe *cqe = NULL;
	struct user_data *data = NULL;

	number_of_requests = nof_requests;
	number_of_accepts = number_of_requests;

	sfd = init_socket(1, 0);

	if (io_uring_queue_init(LISTEN_BACKLOG, &ring, IORING_SETUP_SQPOLL)) {
		handle_error("io_uring_queue_init");
	}

	prep_accept(&ring, sfd);

	while (number_of_requests > 0) {

		if (io_uring_wait_cqe(&ring, &cqe)) {
			handle_error("io_uring_wait_cqe");
		}

		if (cqe->res < 0) {
			(void)fprintf(stderr, "I/O error: %s\n",
			    strerror(-cqe->res));
			exit(EXIT_FAILURE);
		}

		data = io_uring_cqe_get_data(cqe);

		switch (data->io_op) {
		case IORING_OP_ACCEPT:
			prep_recv(&ring, sfd, cqe->res, data->index);
			break;
		case IORING_OP_RECV:
			prep_read(&ring, data->index);
			break;
		case IORING_OP_READ:
			prep_send(&ring, data->index);
			break;
		case IORING_OP_SEND:
			close(data_arr[data->index].socket_fd);
			number_of_requests--;
			break;
		default:
			handle_error("Unknown I/O");
		}
		io_uring_cqe_seen(&ring, cqe);
	}

	io_uring_queue_exit(&ring);
	close(sfd);
}
