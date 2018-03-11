# makefile

CC = g++
CFLAGS = -g -w -Wall -O1 -std=c++11
all: dataserver client

reqchannel.o: reqchannel.h reqchannel.cpp
	$(CC) $(CFLAGS) -c reqchannel.cpp

SafeBuffer.o: SafeBuffer.h SafeBuffer.cpp
	$(CC) $(CFLAGS) -c SafeBuffer.cpp

dataserver: dataserver.cpp reqchannel.o
	$(CC) $(CFLAGS) -o dataserver dataserver.cpp reqchannel.o -lpthread -lrt

client: client.cpp reqchannel.o SafeBuffer.o
	$(CC) $(CFLAGS) -o client client.cpp reqchannel.o SafeBuffer.o -lpthread -lrt

clean:
	rm -rf *.o fifo* dataserver client
