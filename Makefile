CC=gcc
CFLAGS=-Wall -O2
OBJS=arithmetic_encoding.o arithmetic_decoding.o bitbuffer.o cumulative_sums_tree.o
TOBJS=test.o $(OBJS)
MOBJS=arith.o $(OBJS)
#OBJS=test.o arithmetic_encoding_inv.o arithmetic_decoding_inv.o bitbuffer.o cumulative_sums_tree.o  #inverting


all: arith test

arith: $(MOBJS)
	$(CC) $(CFLAGS) -o arith $(MOBJS)

test: $(TOBJS)
	$(CC) $(CFLAGS) -o test $(TOBJS)

test.o: test.c arithmetic_encoding.h arithmetic_decoding.h
	$(CC) $(CFLAGS) -c test.c
	
arithmetic_encoding.o: arithmetic_encoding.c bitbuffer.h cumulative_sums_tree.h
	$(CC) $(CFLAGS) -c arithmetic_encoding.c

arithmetic_decoding.o: arithmetic_decoding.c bitbuffer.h cumulative_sums_tree.h
	$(CC) $(CFLAGS) -c arithmetic_decoding.c
	
arithmetic_encoding_inv.o: arithmetic_encoding_inv.c bitbuffer.h cumulative_sums_tree.h
	$(CC) $(CFLAGS) -c arithmetic_encoding_inv.c

arithmetic_decoding_inv.o: arithmetic_decoding_inv.c bitbuffer.h cumulative_sums_tree.h
	$(CC) $(CFLAGS) -c arithmetic_decoding_inv.c

bitbuffer.o: bitbuffer.c bitbuffer.h
	$(CC) $(CFLAGS) -c bitbuffer.c
	
cumulative_sums_tree.o: cumulative_sums_tree.c
	$(CC) $(CFLAGS) -c cumulative_sums_tree.c

bittry: bittry.o bitbuffer.o
	$(CC) $(CFLAGS) -o bittry bittry.o bitbuffer.o

bittry.o: bittry.c bitbuffer.h
	$(CC) $(CFLAGS) -c bittry.c

clean:
	-rm test *.o decoded compressed
