OBJS = hermes.o udp.o
CC = gcc
CFLAGS = -c -Wall

all: $(OBJS)
	$(CC) $(OBJS) -o hermes

hermes.o: hermes.c udp.h
	$(CC) $(CFLAGS) hermes.c

udp.o: udp.h udp.c
	$(CC) $(CFLAGS) udp.c	

clean:
	rm *.o hermes
