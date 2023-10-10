# cp_liburing

We are demonstrating the __cp__ operation implemented in liburing but it can support multiple requests.

The idea behind is that we create a queue with the requests and the kernel will batch resolve them and thus we are using less system calls as it was with the synchronous implementation