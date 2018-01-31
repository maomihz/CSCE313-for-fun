# makefile

all: memtest

ackerman.o: ackerman.c
	g++ -c -g ackerman.c

my_allocator.o : my_allocator.c
	g++ -c -g my_allocator.c

memtest.o : memtest.c
	g++ -c -g memtest.c

memtest: memtest.o ackerman.o my_allocator.o
	g++ -o memtest memtest.o ackerman.o my_allocator.o
