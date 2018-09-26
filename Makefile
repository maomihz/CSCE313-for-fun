CC = g++
CFLAGS = -g -std=c++11

OBJS = Main.o parser.o command.o

all: main

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

main: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

command.cpp: command.h

run: main
	./$<

test: main
	./$< -t

clean:
	$(RM) $(OBJS) main
