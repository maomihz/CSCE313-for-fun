# makefile
CC = gcc
TARGET = memtest

.PHONY: all run clean

all: $(TARGET)

%.o: %.c
	$(CC) -c -g -o $@ $<


ackerman.o: ackerman.c ackerman.h
my_allocator.o : my_allocator.c my_allocator.h
memtest.o : memtest.c


$(TARGET): memtest.o ackerman.o my_allocator.o
	$(CC) -o $@ $^

run: $(TARGET)
	@./$(TARGET)

clean:
	$(RM) *.o $(TARGET)
