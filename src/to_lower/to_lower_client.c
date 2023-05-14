#include <sys/socket.h>
#include <sys/un.h>

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "to_lower_client.h"

#define TEST "GuInGa GuInGa"
#define CLIENT "help"
#define SERVER "lower"
#define MAX 512

void
to_lower_client(void)
{
	int sock;	  // initialize socket
	char buffer[MAX]; // initialze buffer
	assert((sock = socket(AF_UNIX, SOCK_DGRAM, 0)) != -1); // define buffer

	struct sockaddr_un name = { AF_UNIX, CLIENT }; // define client socket

	assert(bind(sock, (struct sockaddr *)&name,
		   sizeof(struct sockaddr_un)) != -1);	 // bind client socket
	struct sockaddr_un server = { AF_UNIX, SERVER }; // define server socket
	unsigned int size = sizeof(struct sockaddr_un);	 // get size of socket

	int n = sizeof(TEST); // get size of message

	assert(sendto(sock, TEST, n, 0, (struct sockaddr *)&server, size) !=
	    -1); // send message to server socket
	n = recvfrom(sock, buffer, MAX - 1, 0, (struct sockaddr *)&server,
	    &size); // receive message from server socket
	if (n == -1) {
		perror("server");
	}

	buffer[n] = 0;
	printf("Client received: %s\n", buffer); // printf received string
	unlink(CLIENT);				 // unlink socket
}
