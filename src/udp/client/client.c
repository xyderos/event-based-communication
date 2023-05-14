#include "../lib/lib.h"
#include "client.h"

void
udp_client(void)
{
	int sfd = udp_open("localhost", "10000", 0);
	ssize_t nread;
	char buf[BUFFER_SIZE] = "first hello";

	for (size_t i = 0; i < 2; i++) {
		printf("Send: %s\n", buf);
		if (udp_write(sfd, buf, strlen(buf), NULL, 0) != strlen(buf)) {
			fprintf(stderr, "partial/failed write\n");
			exit(EXIT_FAILURE);
		}

		nread = udp_read(sfd, buf, NULL, 0);

		if (nread == -1) {
			perror("read");
			exit(EXIT_FAILURE);
		}

		printf("Received %zd bytes: %s\n", nread, buf);
		strncpy(buf, "hello", 6);
	}

	close(sfd);
	close_semaphore();
}
