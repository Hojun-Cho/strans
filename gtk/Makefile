CFLAGS = -Wall -O2 -I.. `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0`
DSTDIR = /usr/lib64/gtk-3.0/3.0.0/immodules

im-strans.so: main.c
	$(CC) -shared -fPIC $(CFLAGS) -o $@ $< $(LDFLAGS)

docker: main.c Dockerfile
	docker build -t strans-gtk .
	docker run --rm -v $(CURDIR)/..:/src -w /src/gtk strans-gtk make im-strans.so

install:
	cp im-strans.so $(DSTDIR)/
	gtk-query-immodules-3.0 --update-cache

uninstall:
	rm -f $(DSTDIR)/im-strans.so
	gtk-query-immodules-3.0 --update-cache

clean:
	rm -f im-strans.so
