CFLAGS = -Wall -O2 -I.. `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0`
DSTDIR ?= $(shell find /usr/lib* -type d -path "*/gtk-3.0/*/immodules" 2>/dev/null | head -1)

im-strans.so: main.c
	$(CC) -shared -fPIC $(CFLAGS) -o $@ $< $(LDFLAGS)

docker: main.c Dockerfile
	docker build -t strans-gtk .
	docker run --rm -v $(CURDIR)/..:/src -w /src/gtk strans-gtk make im-strans.so

install:
	mkdir -p $(DSTDIR)
	cp im-strans.so $(DSTDIR)/
	gtk-query-immodules-3.0 --update-cache

uninstall:
	rm -f $(DSTDIR)/im-strans.so
	gtk-query-immodules-3.0 --update-cache

clean:
	rm -f im-strans.so
