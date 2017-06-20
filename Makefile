OBJS = fuzz.o
CC = gcc
CFLAGS = -c -Wall

all: $(OBJS)
	$(CC) $(OBJS) -o fuzz

fuzz.o: fuzz.c
	$(CC) $(CFLAGS) fuzz.c

clean:
	rm *.o fuzz
