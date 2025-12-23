CC = cc
CFLAGS = -Wall -Wextra -O2 -fPIC -Isrc -I. -Iuthash -Isrc/xlibi18n -I../xcb-util
AR = ar

SRCS = src/imdkit.c src/encoding.c src/protocolhandler.c \
       src/ximproto.c src/parser.c src/message.c src/common.c \
       src/xlibi18n/lcCharSet.c src/xlibi18n/lcCT.c src/xlibi18n/lcUTF8.c
OBJS = $(SRCS:.c=.o)

all: libxcb-imdkit.a

libxcb-imdkit.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) libxcb-imdkit.a

.PHONY: all clean
