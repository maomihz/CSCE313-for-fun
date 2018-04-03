# makefile
CC = g++
CFLAGS = -g -std=c++11
LDFLAGS = -lpthread -lrt

all: dataserver client

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

%: %.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

reqchannel.o: reqchannel.cpp reqchannel.h
bounded_buffer.o: bounded_buffer.cpp bounded_buffer.h semaphore.h
safecounter.o: safecounter.cpp safecounter.h
test.o: test.cpp bounded_buffer.h
dataserver.o: dataserver.cpp reqchannel.h
client.o: client.cpp reqchannel.h bounded_buffer.h safecounter.h

dataserver: dataserver.o reqchannel.o
client: client.o reqchannel.o bounded_buffer.o safecounter.o
test: test.o bounded_buffer.o


clean:
	$(RM) *.o fifo* dataserver client test
