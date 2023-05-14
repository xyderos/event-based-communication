#include <sys/socket.h>

#include <netinet/ip.h>

#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "echo_client.h"

#define PORT 8080
#define MAX 512
#define TEST "Hello, hello"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

void
echo_client(void)
{
	long sock = 0, n = sizeof(TEST);
	char buffer[MAX];
	struct sockaddr_in name, server;
	unsigned int size = sizeof(struct sockaddr_in);

	assert((sock = socket(AF_INET, SOCK_DGRAM, 0)) != -1);

	name.sin_family = AF_INET;
	name.sin_port = 0;
	name.sin_addr.s_addr = htonl(INADDR_ANY);

	assert(bind(sock, (struct sockaddr *)&name, sizeof(name)) != -1);

	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT);
	server.sin_addr.s_addr = inet_addr(SERVER_IP);

	assert(sendto((int)sock, TEST, (unsigned long)n, 0,
		   (struct sockaddr *)&server, size) != -1);
	n = recvfrom((int)sock, buffer, MAX - 1, 0, (struct sockaddr *)&server,
	    &size);
	if (n == -1) {
		perror("server");
	}

	buffer[n] = 0;
	printf("Client received: %s\n", buffer);
	exit(0);
}
