#include <pthread.h>
#include <unistd.h>

#include "connection.h"

void *send_requests(void *);

void generate_requests(unsigned int, const char *const);
