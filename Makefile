OBJS = hermes.o
CC = gcc
CFLAGS = -c -Wall

all: $(OBJS)
	$(CC) $(OBJS) -o hermes

hermes.o: hermes.c udp.h
	$(CC) $(CFLAGS) hermes.c

clean:
	rm *.o hermes
