# poll

We set up a kernel thread to poll the shared submission queue. The kernel thread will see the added entry and will execute it without calling the __io_uring_enter__ system call.

Useful to share a queue between the user space and kernel space

Note that the kernel polling queue is CPU intensive even when no submissions are happening