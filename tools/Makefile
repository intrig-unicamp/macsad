CC=gcc
CFLAGS=-Wall -pthread
LIB=

all: sendRaw

sendRaw:
	$(CC) $(CFLAGS) $(LIB) sendRawIP.c -o sendRawIP
#	$(CC) $(CFLAGS) $(LIB) vlan_send.c -o sendRaw

clean:
	rm -f sendRaw

