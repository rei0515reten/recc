CFLAGS=-std=c11 -g3 -O0 -static
SRCS=$(wildcard src/*.c)
SRCS:=$(filter-out src/recc.c src/test.c,$(SRCS))
OBJS=$(SRCS:.c=.o)

recc: $(OBJS)
				$(CC) -o recc $(OBJS) $(LDFLAGS)

$(OBJS): src/recc.h

test: recc
	./test.sh

clean:
		rm -f recc src/*.o *~ tmp*

.PHONY: test clean
