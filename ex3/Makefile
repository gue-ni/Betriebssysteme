# @file Makefile
# @author Jakob G. Maier <e11809618@student.tuwien.ac.at>
# @date 10.01.2020
# Makefile for generator and supervisor

CC 	   = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
LFLAGS = -lrt -pthread

PROGS  = supervisor generator
DEPS   = common

.PHONY: all clean

all: $(PROGS)

$(PROGS): %: %.o $(DEPS:=.o)
	$(CC) -o $@ $^ $(LFLAGS)

%.o: %.c $(DEPS:=.h)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o $(PROGS)
