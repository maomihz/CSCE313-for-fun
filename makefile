# makefile
CC = g++
TARGET = memtest

.PHONY: all run clean

all: memtest

%.o: %.c
	$(CC) -c -g -o $@ $<


ackerman.o: ackerman.c
my_allocator.o : my_allocator.c
memtest.o : memtest.c


$(TARGET): memtest.o ackerman.o my_allocator.o
	$(CC) -o $@ $^

run: $(TARGET)
	./$(TARGET)

clean:
	$(RM) *.o $(TARGET)
