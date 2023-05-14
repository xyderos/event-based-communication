#define _GNU_SOURCE

#include <assert.h>

#include "../lib/lib.h"
#include "server.h"

void
udp_server(void)
{
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len;
	ssize_t nread;
	char buf[BUFFER_SIZE];
	int sfd = udp_open(NULL, "10000", 1), s;
	assert(sfd > -1);

	for (;;) {
		peer_addr_len = sizeof(struct sockaddr_storage);

		nread = udp_read(sfd, buf, (struct sockaddr *)&peer_addr,
		    &peer_addr_len);
		if (nread == -1) {
			continue;
		}

		char host[NI_MAXHOST], service[NI_MAXSERV];

		s = getnameinfo((struct sockaddr *)&peer_addr, peer_addr_len,
		    host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
		if (s == 0)
			printf("Received %zd bytes from %s:%s: %s\n", nread,
			    host, service, buf);
		else
			(void)fprintf(stderr, "getnameinfo: %s\n",
			    gai_strerror(s));
	}
}
