CC=gcc
CFLAGS=-Wall -pthread -std=c99
LIB=

all: test_messages test_backend maco_controller maco_l3_controller maco_l3-full_controller

test_messages:
	$(CC) $(CFLAGS) $(LIB) sock_helpers.c fifo.c threadpool.c ctrl_plane_backend.c handlers.c messages.c test_messages.c -o test_messages

test_backend: 
	$(CC) $(CFLAGS) $(LIB) handlers.c ctrl_plane_backend.c messages.c sock_helpers.c threadpool.c fifo.c test_backend.c -o test_backend

maco_controller:
	$(CC) $(CFLAGS) $(LIB) handlers.c controller.c messages.c sock_helpers.c threadpool.c fifo.c odp_controller.c -o maco_controller

maco_l3_controller:
	$(CC) $(CFLAGS) $(LIB) handlers.c controller.c messages.c sock_helpers.c threadpool.c fifo.c odp_l3_controller.c -o maco_l3_controller

maco_l3-full_controller:
	$(CC) $(CFLAGS) $(LIB) handlers.c controller.c messages.c sock_helpers.c threadpool.c fifo.c odp_l3-full_controller.c -o maco_l3-full_controller



clean:
	rm -f test_messages test_backend maco_controller maco_l3_controller maco_l3-full_controller

