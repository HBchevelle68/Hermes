OBJS = udpFuzz.o
CC = gcc
CFLAGS = -c -Wall

all: $(OBJS)
	$(CC) $(OBJS) -o udpFuzz

udpFuzz.o: udpFuzz.c
	$(CC) $(CFLAGS) udpFuzz.c


clean:
	rm *.o udpFuzz


OBJS = udpFuzz.o
CC = gcc
CFLAGS = -c -Wall

all: $(OBJS)
	$(CC) $(OBJS) -o raw

udpFuzz.o: udpFuzz.c
	$(CC) $(CFLAGS) raw.c


clean:
	rm *.o raw