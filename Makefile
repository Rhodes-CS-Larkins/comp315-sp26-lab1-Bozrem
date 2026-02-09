CC=clang
CFLAGS=-g -Wall -Werror -gdwarf-4

OBJS = util.o

.PHONY=all
all: ping pong

ping: ping.o $(OBJS)
	$(CC) -o $@ $^

pong: pong.o $(OBJS)
	$(CC) -o $@ $^

ping-master: ping-master.o $(OBJS)
	$(CC) -o $@ $^

pong-master: pong-master.o $(OBJS)
	$(CC) -o $@ $^

clean:
	rm -f ping pong *.o
