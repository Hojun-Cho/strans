CC = 9c
LD = 9l
CFLAGS = -Wall -Wextra -O2 -g
PROG = strans

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

all: $(PROG) xim bench 

$(PROG): $(OBJS)
	$(LD) -o $@ $(OBJS) -lthread -lString -lbio -lxcb -lm

$(OBJS): dat.h fn.h ipc.h

clean:
	rm -f $(OBJS) $(PROG)
	make -C xim/ clean
	make -C bench/ clean

xim:
	make -C xim/

bench:
	make -C bench/

.PHONY: all clean xim bench
