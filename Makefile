DIRS = $(wildcard day*)
SRCS = $(patsubst %, %/solution.c, $(DIRS))
BINS = $(patsubst %, %/solution, $(DIRS))
BINS_FAST = $(patsubst %, %/solution_fast, $(DIRS))
CC = gcc
CFLAGS =

all: libadvent.a $(BINS) $(BINS_FAST)

libadvent.a: advent.o
	ar rcs $@ $<

advent.o: advent.c advent.h gstd/* trashhash.h
	$(CC) -I gstd -c -Wall -Wextra -g -o $@ $<

%: %.c libadvent.a
	$(CC) $(CFLAGS) -I. -I gstd -Wall -Wextra -g -o $@ $< libadvent.a -lm

%_fast: %.c libadvent.a
	$(CC) $(CFLAGS) -I. -I gstd -Wall -Wextra -g -O3 -o $@ $< libadvent.a -lm

clean:
	-rm libadvent.a
	-rm advent.o
	-rm $(BINS) $(BINS_FAST)
