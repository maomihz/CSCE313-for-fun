# makefile

all: dataserver client

reqchannel.o: reqchannel.h reqchannel.cpp
	g++ -std=c++11 -c -g reqchannel.cpp

bounded_buffer.o: bounded_buffer.h bounded_buffer.cpp
	g++ -std=c++11 -c -g bounded_buffer.cpp

dataserver: dataserver.cpp reqchannel.o 
	g++ -std=c++11 -g -o dataserver dataserver.cpp reqchannel.o -lpthread

client: client.cpp reqchannel.o bounded_buffer.o
	g++ -std=c++11 -g -o client client.cpp reqchannel.o bounded_buffer.o -lpthread

clean:
	rm -rf *.o fifo* dataserver client
