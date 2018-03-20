# makefile
CC = g++
CFLAGS = -g -std=c++11
LDFLAGS = -lpthread

all: dataserver client

reqchannel.o: reqchannel.cpp reqchannel.h
	$(CC) $(CFLAGS) -c -o $@ $<

bounded_buffer.o: bounded_buffer.cpp bounded_buffer.h
	$(CC) $(CFLAGS) -c -o $@ $<

dataserver: dataserver.cpp reqchannel.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

client: client.cpp reqchannel.o bounded_buffer.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) *.o fifo* dataserver client
