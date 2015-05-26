
CC=gcc
CFLAGS=-g -D_GNU_SOURCE
DEPS = orpfs.h orpio.h dir_table.h tidl.h xtsProto.h
OBJS = orpfs.o orpio.o dir_table.o tidl.o xtsProto.c

%.o: %.c $(DEPS)
	$(CC) -Wall -c -o $@ $< $(CFLAGS) `pkg-config --cflags fuse`

orpfs: $(OBJS)
	$(CC) -Wall -o orpfs $(OBJS) `pkg-config --libs fuse`
