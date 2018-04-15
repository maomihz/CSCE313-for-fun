# makefile
CC = g++
CFLAGS = -g -std=c++11 -Wno-nonnull
LDFLAGS = -lpthread -lrt

all: dataserver client
mq: mqdataserver mqclient

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

%: %.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

reqchannel.o: reqchannel.cpp reqchannel.h
mqreqchannel.o: mqreqchannel.cpp mqreqchannel.h
bounded_buffer.o: bounded_buffer.cpp bounded_buffer.h semaphore.h
safecounter.o: safecounter.cpp safecounter.h
test.o: test.cpp bounded_buffer.h
dataserver.o: dataserver.cpp reqchannel.h
mqdataserver.o: mqdataserver.cpp mqreqchannel.h
client.o: client.cpp reqchannel.h bounded_buffer.h safecounter.h
mqclient.o: mqclient.cpp mqreqchannel.h bounded_buffer.h safecounter.h


dataserver: dataserver.o reqchannel.o
mqdataserver: mqdataserver.o mqreqchannel.o
client: client.o reqchannel.o bounded_buffer.o safecounter.o
mqclient: mqclient.o mqreqchannel.o bounded_buffer.o safecounter.o
test: test.o bounded_buffer.o


mqdataserver.cpp: dataserver.cpp
	sed 's/"reqchannel.h"/"mqreqchannel.h"/g' $< >$@

mqclient.cpp: client.cpp
	sed -e 's/"reqchannel.h"/"mqreqchannel.h"/g' -e 's/"dataserver"/"mqdataserver"/g' $< >$@



clean:
	$(RM) *.o fifo* dataserver client mqdataserver mqclient test mqdataserver.cpp mqclient.cpp
