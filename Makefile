CC = 9c
LD = 9l
CFLAGS = -Wall -Wextra -O2
PROG = strans

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

all: $(PROG)

$(PROG): $(OBJS)
	$(LD) -o $@ $(OBJS) -lthread -lString -lbio -lxcb -lm

$(OBJS): dat.h fn.h ipc.h

clean:
	rm -f $(OBJS) $(PROG)

.PHONY: all clean
