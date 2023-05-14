#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LISTEN_BACKLOG 80
#define SOCKET_PORT 8080
#define handle_error(msg)           \
	do {                        \
		perror(msg);        \
		exit(EXIT_FAILURE); \
	} while (0)

int init_socket(int, int);
