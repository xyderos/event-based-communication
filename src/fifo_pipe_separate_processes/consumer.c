#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "consumer.h"

#define ITERATIONS 10
#define BURST 1000

void
fifo_pipe_consumer(void)
{
	int descr[2], flag = O_RDONLY, pipe = open("sesame", flag);

	for (int i = 0; i < ITERATIONS; i++) {
		for (int j = 0; j < BURST; j++) {
			int buffer[10];
			read(descr[0], &buffer, 10);
		}
		printf("read %d buffer done \n", i);
		sleep(1);
	}
	printf("consumer done\n");
}
