#	@author: Jakob G. Maier <e11809618@student.tuwien.ac.at>
#	@date: 2019-11-17
#	@brief: Makefile for server and client
CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

PROGS = client server 
DEPS = utils

.PHONY: all clean

all: $(PROGS)
	rm -rf *.o

$(PROGS): % : %.o $(DEPS:=.o)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c $(DEPS:=.h)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o $(PROGS)

