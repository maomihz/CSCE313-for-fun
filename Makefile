CC = g++
CFLAGS = -g -std=c++11 -Wall -Wextra

OBJS = Main.o parser.o command.o

all: main

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

main: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lreadline

command.cpp: command.h

run: main
	./$<

test: main
	./$< -t

clean:
	$(RM) $(OBJS) main
