#include "connection.h"

int
init_socket(int is_server, int nonblock)
{
	struct sockaddr_in addr;
	int socket_flags = SOCK_STREAM, sfd = 0;

	if (nonblock)
		socket_flags |= SOCK_NONBLOCK;

	sfd = socket(AF_INET, socket_flags, 0);

	if (sfd == -1) {
		handle_error("socket");
	}

	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(SOCKET_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	if (is_server) {
		const int optval = 1;

		if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval,
			sizeof(optval))) {
			handle_error("setsocketopt");
		}

		if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
			handle_error("bind");
		}

		if (listen(sfd, LISTEN_BACKLOG) == -1) {
			handle_error("listen");
		}

	} else if (connect(sfd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
		handle_error("connect");
	}

	return sfd;
}
