CC = cc
CFLAGS = -Wall -Wextra -O2 -I.. -Ixcb-imdkit/src
PROG = strans-xim

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

LIBS = xcb-imdkit/libxcb-imdkit.a xcb-util/libxcb-util.a

all: $(PROG)

$(PROG): $(OBJS) $(LIBS)
	$(CC) -o $@ $(OBJS) $(LIBS) -lxcb

$(OBJS): ../ipc.h

xcb-imdkit/libxcb-imdkit.a: xcb-util/libxcb-util.a
	$(MAKE) -C xcb-imdkit

xcb-util/libxcb-util.a:
	$(MAKE) -C xcb-util

clean:
	rm -f $(OBJS) $(PROG)
	$(MAKE) -C xcb-imdkit clean
	$(MAKE) -C xcb-util clean

.PHONY: all clean
