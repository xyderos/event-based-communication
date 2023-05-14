#include <pthread.h>
#include <unistd.h>

#include "client.h"
#include "connection.h"

void *
send_requests(void *arg)
{
	char *file_name = (char *)arg, buf[BUFSIZ] = "";
	int sfd = init_socket(0, 0);

	if (send(sfd, file_name, strlen(file_name), 0) == -1) {
		handle_error("send");
	}

	if (recv(sfd, buf, BUFSIZ, 0) == -1) {
		handle_error("recv");
	}

	printf("%s\n", buf);

	close(sfd);

	return NULL;
}

void
generate_requests(unsigned int nof_threads, const char *const file_path)
{
	if (nof_threads > 1) {

		pthread_t threads[nof_threads];

		for (unsigned i = 0; i < nof_threads; i++) {
			if (pthread_create(&threads[i], NULL, send_requests,
				file_path)) {
				handle_error("pthread_create");
			}
		}
		for (unsigned i = 0; i < nof_threads; i++) {
			if (pthread_join(threads[i], NULL)) {
				handle_error("pthread_join");
			}
		}
	} else
		send_requests(file_path);
}
