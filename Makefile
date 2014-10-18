CC=gcc
CFLAGS=-Wall
LIBS=-lpthread

all: quicksort

quicksort: quicksort.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean all

clean:
	rm -f *.o quicksort
