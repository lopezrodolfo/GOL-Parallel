CC = gcc
CFLAGS = -g -Wall -Wextra -std=c11
LDFLAGS = -lncurses -lpthread

TARGETS = gol

GOL_LIB=gol.o
BARRIER_LIB=barrier.o

all: $(TARGETS)

gol: main.c $(GOL_LIB) $(BARRIER_LIB)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(GOL_LIB): gol.c gol.h
	$(CC) -c $(CFLAGS) $<

$(BARRIER_LIB): barrier.c barrier.h
	$(CC) -c $(CFLAGS) $<

clean:
	$(RM) $(TARGETS) $(GOL_LIB) $(BARRIER_LIB)
