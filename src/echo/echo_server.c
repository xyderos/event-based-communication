#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/ip.h>

#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "echo_server.h"

#define PORT 8080
#define MAX 512

void
echo_server(void)
{
	int sock = 0;
	struct sockaddr_in name;
	struct sockaddr_in client;
	unsigned int size = sizeof(struct sockaddr_in);
	char buffer[MAX];

	/* A socket is created */
	assert((sock = socket(AF_INET, SOCK_DGRAM, 0)) != -1);

	name.sin_family = AF_INET;
	name.sin_port = htons(PORT);
	name.sin_addr.s_addr = htonl(INADDR_ANY);

	assert(bind(sock, (struct sockaddr *)&name, sizeof(name)) != -1);

	while (1) {
		ssize_t n = 0;
		n = recvfrom(sock, buffer, MAX - 1, 0,
		    (struct sockaddr *)&client, &size);
		if (n == -1) {
			perror("server");
		}
		buffer[n] = 0;

		printf("Server received: %s\n", buffer);
		printf("Server: from destination %s %d\n",
		    inet_ntoa(client.sin_addr), ntohs(client.sin_port));

		assert(sendto(sock, buffer, (unsigned long)n, 0,
			   (struct sockaddr *)&client, size) == n);
	}
}
