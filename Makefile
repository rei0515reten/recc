CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

recc: $(OBJS)
				$(CC) -o recc $(OBJS) $(LDFLAGS)

$(OBJS): recc.h

test: recc
	./test.sh

clean: rm -f recc *.o *~ tmp*

.PHONY: test clean
