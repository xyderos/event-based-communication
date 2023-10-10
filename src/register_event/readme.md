# register event

IO_uring posts an event whenever a completion occurs. This extends the behaviour to __poll and epoll__ so the program will be busy processing the event loop instead of being blocked