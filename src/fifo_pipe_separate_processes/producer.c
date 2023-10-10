#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "producer.h"

#define ITERATIONS 10
#define BURST 1000

void
fifo_pipe_producer(void)
{
	int descr[2], flag = O_WRONLY;

	unsigned int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	mkfifo("sesame", mode);

	(void)open("sesame", flag);

	for (int i = 0; i < ITERATIONS; i++) {
		for (int j = 0; j < BURST; j++) {
			write(descr[1], "0123456789", 10);
		}
		printf("producer burst %d done\n", i);
	}

	printf("producer done\n");
}
