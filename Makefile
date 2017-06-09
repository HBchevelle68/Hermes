OBJS = udpFuzz.o
CC = gcc
CFLAGS = -c -Wall

all: $(OBJS)
	$(CC) $(OBJS) -o udpFuzz

udpFuzz.o: udpFuzz.c
	$(CC) $(CFLAGS) udpFuzz.c

clean:
	rm *.o udpFuzz
