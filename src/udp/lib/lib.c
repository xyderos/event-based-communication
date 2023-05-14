#include "lib.h"

static char *client_sem_name = "/client_sem";
static sem_t *client_sem;
static struct timespec ts;

int
udp_open(char *hostName, char *port, int server)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd = 0, s;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	s = getaddrinfo(hostName, port, &hints, &result);

	if (s != 0) {
		(void)fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;

		if (server && bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) {
			break;
		}

		else if (!server &&
		    connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
			break;
		}

		close(sfd);
	}

	if (!rp) {
		char *error_message = server ? "Could not bind" :
					       "Could not connect";
		(void)fprintf(stderr, "%s\n", error_message);
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);

	client_sem = sem_open(client_sem_name, O_CREAT, S_IRWXU, 1);

	if (!server && client_sem == SEM_FAILED) {
		perror("sem_open");
		exit(EXIT_FAILURE);
	}

	return sfd;
}

ssize_t
udp_write(int sfd, char *buffer, int nread, struct sockaddr *peer_addr,
    int peer_addr_len)
{
	int s = 0, is_ask_request;

	if (nread > BUFFER_SIZE) {
		(void)fprintf(stderr, "Exceed max buffer size\n");
		exit(EXIT_FAILURE);
	}

	if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
		perror("clock_gettime");
		exit(EXIT_FAILURE);
	}

	ts.tv_sec += TIMEOUT_SECONDS;
	// wait ack
	errno = 0;

	is_ask_request = 0;
	if ((is_ask_request = strncmp(buffer, "ack", 4)) != 0)
		s = sem_timedwait(client_sem, &ts);

	if (s == -1) {
		if (errno == ETIMEDOUT) {
			return udp_write(sfd, buffer, nread, peer_addr,
			    peer_addr_len);
		} else {
			perror("sem_timedwait");
			exit(EXIT_FAILURE);
		}
	} else {
		if (is_ask_request != 0) {
			sem_post(client_sem);
		}
		return sendto(sfd, buffer, nread, 0, peer_addr, peer_addr_len);
	}
}

ssize_t
udp_read(int sfd, char *buffer, struct sockaddr *peer_addr,
    socklen_t *peer_addr_len)
{
	ssize_t recv_bytes;
	memset(buffer, 0, BUFFER_SIZE);

	recv_bytes = recvfrom(sfd, buffer, BUFFER_SIZE, 0, peer_addr,
	    peer_addr_len);

	if (recv_bytes > 0 && strncmp(buffer, "ack", 4) != 0) {
		printf("Send ack\n");
		udp_write(sfd, "ack", 4, peer_addr, *peer_addr_len);
	}

	return recv_bytes;
}

void
close_semaphore()
{
	if (sem_close(client_sem)) {
		perror("sem_close");
		exit(EXIT_FAILURE);
	}
	if (sem_unlink(client_sem_name)) {
		perror("sem_close");
		exit(EXIT_FAILURE);
	}
}
