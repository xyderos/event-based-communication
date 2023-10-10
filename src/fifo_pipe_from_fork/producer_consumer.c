#include <sys/wait.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "producer_consumer.h"

#define ITERATIONS 10
#define BURST 10

int
forked_producer_consumer_array(void)
{
	int descr[2];

	assert(0 == pipe(descr));
	if (fork() == 0) {
		for (int i = 0; i < ITERATIONS; i++) {
			for (int j = 0; j < BURST; j++) {
				int buffer[10];
				read(descr[0], &buffer, 10);
			}
			printf("buffer %d read\n", i);
			sleep(1);
		}
		printf("consumer done\n");
		return 0;
	}

	for (int i = 0; i < ITERATIONS; i++) {
		for (int j = 0; j < BURST; j++) {
			write(descr[1], "0123456789", 10);
		}
		printf("producer burst %d done\n", i);
	}
	printf("producer done\n");

	wait(NULL);
	printf("all done\n");
	return 0;
}

int
forked_producer_consumer_double(void)
{
	int descr[2];

	assert(0 == pipe(descr));
	if (fork() == 0) {
		/* consumer */
		for (int i = 0; i < ITERATIONS; i++) {
			double buffer;
			read(descr[0], &buffer, sizeof(double));
			printf("received %f\n", buffer);
			sleep(1);
		}
		printf("consumer done\n");
		return 0;
	}
	for (int i = 0; i < ITERATIONS; i++) {
		double pi = 3.14 * i;
		write(descr[1], &pi, sizeof(double));
	}
	printf("producer done\n");
	wait(NULL);
	printf("all done\n");
	return 0;
}
