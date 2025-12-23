#include "dat.h"
#include "fn.h"

static char adir[40];

static void
sendkey(int fd, u32int ks, u32int mod)
{
	Keyreq kr;

	kr.fd = fd;
	kr.ks = ks;
	kr.mod = mod;
	chansend(keyc, &kr);
}

static void
clientthread(void *arg)
{
	int fd;
	uchar req[4];

	fd = (int)(uintptr)arg;
	threadsetname("client %d", fd);
	while(read(fd, req, 4) == 4)
		sendkey(fd, req[2] | (req[3] << 8), req[1]);
	close(fd);
}

static void
srvinit(void)
{
	char addr[64];

	snprint(addr, sizeof(addr), "unix!/tmp/strans.%d", getuid());
	remove(addr + 5);
	if(announce(addr, adir) < 0)
		die("announce: %r");
}

void
srvthread(void*)
{
	char ldir[40];
	int fd;

	threadsetname("srv");
	srvinit();
	for(;;){
		fd = listen(adir, ldir);
		if(fd < 0)
			continue;
		proccreate(clientthread, (void*)(uintptr)fd, 8192);
	}
}
