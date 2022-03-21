CFLAGS=-std=c11 -g3 -O0 -static
SRCS=$(wildcard *.c)
SRCS:=$(filter-out recc.c,$(SRCS))
OBJS=$(SRCS:.c=.o)

recc: $(OBJS)
				$(CC) -o recc $(OBJS) $(LDFLAGS)

$(OBJS): recc.h

test: recc
	./test.sh

clean:
		rm -f recc *.o *~ tmp*

.PHONY: test clean
