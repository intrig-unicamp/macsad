CC=gcc
CFLAGS=-Wall -pthread -std=c99
LIB=

#all: test_messages test_backend mac_controller mac_l3_controller mac_l3_nhg_controller
all: mac_controller mac_l2_l3_controller mac_l3_controller mac_l3_nhg_controller 

#test_messages:
#	$(CC) $(CFLAGS) $(LIB) sock_helpers.c fifo.c threadpool.c ctrl_plane_backend.c handlers.c messages.c test_messages.c -o test_messages

mac_controller:
	$(CC) $(CFLAGS) $(LIB) handlers.c controller.c messages.c sock_helpers.c threadpool.c fifo.c mac_controller.c -o mac_controller

mac_l2_l3_controller:
	    $(CC) $(CFLAGS) $(LIB) handlers.c controller.c messages.c sock_helpers.c threadpool.c fifo.c mac_l2_l3_controller.c -o mac_l2_l3_controller

mac_l3_controller:
	    $(CC) $(CFLAGS) $(LIB) handlers.c controller.c messages.c sock_helpers.c threadpool.c fifo.c mac_l3_controller.c -o mac_l3_controller

mac_l3_nhg_controller:
	    $(CC) $(CFLAGS) $(LIB) handlers.c controller.c messages.c sock_helpers.c threadpool.c fifo.c mac_l3_nhg_controller.c -o mac_l3_nhg_controller

clean:
#rm -f test_messages test_backend mac_controller mac_l2_l3controller 
	rm -f mac_controller mac_l2_l3_controller mac_l3_controller mac_l3_nhg_controller 

