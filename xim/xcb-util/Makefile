CC = cc
CFLAGS = -Wall -Wextra -O2 -I. -Isrc -Ixcb

OBJS = src/atoms.o src/event.o src/xcb_aux.o

all: libxcb-util.a

libxcb-util.a: $(OBJS)
	ar rcs $@ $(OBJS)

clean:
	rm -f $(OBJS) libxcb-util.a

.PHONY: all clean
