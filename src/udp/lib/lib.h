#define _POSIX_C_SOURCE 201112L

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_SIZE 65507
#define TIMEOUT_SECONDS 10

int udp_open(char *, char *, int);

ssize_t udp_write(int, char *, int, struct sockaddr *, int);

ssize_t udp_read(int, char *, struct sockaddr *, socklen_t *);

void close_semaphore(void);
