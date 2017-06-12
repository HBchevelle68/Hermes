OBJS = fuzz.o udp.o
CC = gcc
CFLAGS = -c -Wall

all: $(OBJS)
	$(CC) $(OBJS) -o fuzz

fuzz.o: fuzz.c udp.h
	$(CC) $(CFLAGS) fuzz.c

udp.o: udp.h udp.c
	$(CC) $(CFLAGS) udp.c	

clean:
	rm *.o fuzz
