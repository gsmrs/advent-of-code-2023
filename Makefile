DIRS = $(wildcard day*)
SRCS = $(patsubst %, %/solution.c, $(DIRS))
BINS = $(patsubst %, %/solution, $(DIRS))
BINS_FAST = $(patsubst %, %/solution_fast, $(DIRS))
CFLAGS =

all: libadvent.a $(BINS) $(BINS_FAST)

libadvent.a: advent.o
	ar rcs $@ $<

advent.o: advent.c advent.h gstd/*
	gcc -I gstd -c -Wall -Wextra -g -o $@ $<

%: %.c libadvent.a
	gcc $(CFLAGS) -I. -I gstd -Wall -Wextra -g -o $@ $< libadvent.a -lm

%_fast: %.c libadvent.a
	gcc $(CFLAGS) -I. -I gstd -Wall -Wextra -g -O3 -o $@ $< libadvent.a -lm

