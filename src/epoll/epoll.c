#include <sys/epoll.h>
#include <sys/sendfile.h>
#include <sys/stat.h>

#include <errno.h>
#include <time.h>
#include <unistd.h>

#include "epoll.h"

void
epoll(unsigned int number_of_requests)
{
	struct epoll_event ev, evlist[LISTEN_BACKLOG];
	int sfd = init_socket(1, 0), epfd = epoll_create1(0), ready = 0,
	    cfd = 0, fd = 0;
	struct stat statbuf;

	if (epfd == -1) {
		handle_error("epoll_create1");
	}

	ev.events = EPOLLIN;
	ev.data.fd = sfd;

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &ev) == -1) {
		handle_error("epoll_ctl");
	}

	while (number_of_requests > 0) {

		ready = epoll_wait(epfd, evlist, LISTEN_BACKLOG, -1);

		if (ready == -1) {
			if (errno == EINTR) {
				continue;
			}

			else {
				handle_error("epoll_wait");
			}
		}

		for (int i = 0; i < ready; i++) {
			if (evlist[i].events & EPOLLIN) {
				if (evlist[i].data.fd == sfd) {

					cfd = accept(sfd, NULL, NULL);

					if (cfd == -1) {
						handle_error("accept");
					}

					ev.events = EPOLLIN;
					ev.data.fd = cfd;

					if (epoll_ctl(epfd, EPOLL_CTL_ADD, cfd,
						&ev) == -1) {
						handle_error("epoll_ctl");
					}

				} else {
					char buff[BUFSIZ] = "";

					if (recv(evlist[i].data.fd, buff,
						BUFSIZ, 0) == -1) {
						handle_error("recv");
					}

					fd = open(buff, O_RDONLY);

					if (fd == -1) {
						handle_error("open");
					}

					if (fstat(fd, &statbuf) == -1) {
						handle_error("fstat");
					}

					if (sendfile(evlist[i].data.fd, fd,
						NULL,
						(unsigned long)
						    statbuf.st_size) == -1) {
						handle_error("sendfile");
					}

					close(fd);
					close(evlist[i].data.fd);
					number_of_requests--;
				}
			}
		}
	}
	close(sfd);
}
