# makefile
CC = g++
CFLAGS = -g -Wall -O1 -std=c++11

all: dataserver client

%.o: %.cpp
	$(CC) $(CFLAGS) -w -c -o $@ $<

%: %.o
	$(CC) $(CFLAGS) -o $@ $^ -lpthread -lrt

reqchannel.o: reqchannel.cpp reqchannel.h
SafeBuffer.o: SafeBuffer.cpp SafeBuffer.h
Histogram.o: Histogram.cpp Histogram.h
client.o: client.cpp
dataserver.o: dataserver.cpp

dataserver: dataserver.o reqchannel.o 
client: client.o reqchannel.o SafeBuffer.o Histogram.o


clean:
	$(RM) *.o fifo* dataserver client
