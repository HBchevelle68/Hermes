OBJS = raw.o
CC = gcc
CFLAGS = -c -Wall

all: $(OBJS)
	$(CC) $(OBJS) -o raw

raw.o: raw.c
	$(CC) $(CFLAGS) raw.c


clean:
	rm *.o raw