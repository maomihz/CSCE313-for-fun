# makefile
CC = g++
CFLAGS = -g -std=c++11 -Wno-nonnull
LDFLAGS = -lpthread -lrt

all: shm
pipe: dataserver client
mq: mqdataserver mqclient
shm: shmdataserver shmclient

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

%: %.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

reqchannel.o: reqchannel.cpp reqchannel.h
mqreqchannel.o: mqreqchannel.cpp mqreqchannel.h
shmreqchannel.o: shmreqchannel.cpp shmreqchannel.h kernelsemaphore.h
bounded_buffer.o: bounded_buffer.cpp bounded_buffer.h semaphore.h
safecounter.o: safecounter.cpp safecounter.h
test.o: test.cpp bounded_buffer.h
dataserver.o: dataserver.cpp reqchannel.h
mqdataserver.o: mqdataserver.cpp mqreqchannel.h
shmdataserver.o: shmdataserver.cpp shmreqchannel.h
client.o: client.cpp reqchannel.h bounded_buffer.h safecounter.h
mqclient.o: mqclient.cpp mqreqchannel.h bounded_buffer.h safecounter.h
shmclient.o: shmclient.cpp shmreqchannel.h bounded_buffer.h safecounter.h


dataserver: dataserver.o reqchannel.o
mqdataserver: mqdataserver.o mqreqchannel.o
shmdataserver: shmdataserver.o shmreqchannel.o
client: client.o reqchannel.o bounded_buffer.o safecounter.o
mqclient: mqclient.o mqreqchannel.o bounded_buffer.o safecounter.o
shmclient: shmclient.o shmreqchannel.o bounded_buffer.o safecounter.o
test: test.o bounded_buffer.o


mqdataserver.cpp: dataserver.cpp
	sed 's/"reqchannel.h"/"mqreqchannel.h"/g' $< >$@

mqclient.cpp: client.cpp
	sed -e 's/"reqchannel.h"/"mqreqchannel.h"/g' -e 's/"dataserver"/"mqdataserver"/g' $< >$@

shmdataserver.cpp: dataserver.cpp
	sed 's/"reqchannel.h"/"shmreqchannel.h"/g' $< >$@

shmclient.cpp: client.cpp
	sed -e 's/"reqchannel.h"/"shmreqchannel.h"/g' -e 's/"dataserver"/"shmdataserver"/g' $< >$@



clean:
	$(RM) *.o fifo* *dataserver *client test mqdataserver.cpp mqclient.cpp shmdataserver.cpp shmclient.cpp
