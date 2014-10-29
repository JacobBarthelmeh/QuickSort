CC=gcc
CFLAGS=-Wall
LIBS=-lpthread
TCMALLOC=-ltcmalloc -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free

all: quicksort

quicksort: quicksort.o
	$(CC) -O3 -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean all

clean:
	rm -f *.o quicksort
