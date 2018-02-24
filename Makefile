# makefile
CC = g++
CFLAGS = -g -Wall -std=c++11
TARGET = shell

.PHONY: all run clean

all: $(TARGET)

%.o: %.cpp
	$(CC) -c -o $@ $(CFLAGS) $<

shell.o : shell.cpp
command.o: command.cpp command.h
test.o: test.cpp command.h


$(TARGET): shell.o command.o
	$(CC) -o $@ $(CFLAGS) $^

test: test.o command.o
	$(CC) -o $@ $(CFLAGS) $^

runtest: test
	@./test

run: $(TARGET)
	@./$(TARGET)

clean:
	$(RM) *.o $(TARGET) test
