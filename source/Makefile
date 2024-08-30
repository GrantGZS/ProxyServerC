CC=gcc
CFLAGS=-Wall -g
BIN=tcpproxy
OBJS=sockutils.o tcpproxy.o main.o buf.o

.PHONY: clean

$(BIN): $(OBJS)
	$(CC) -o $(BIN) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o $(BIN)

