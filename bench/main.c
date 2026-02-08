#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>

enum {
	Kback	= 0xf008,
	Kret	= 0xf00d,
	Kesc	= 0xf01b,
	Mctrl	= 1<<2,
};

typedef struct Key Key;
struct Key {
	int	k;
	int	mod;
};

static Key *keys;
static int nkeys;
static int fd;

static void
die(char *msg)
{
	perror(msg);
	exit(1);
}

static void
addkey(int k, int mod)
{
	static int cap;

	if(nkeys >= cap){
		cap = cap ? cap * 2 : 256;
		keys = realloc(keys, cap * sizeof(Key));
		if(!keys)
			die("realloc");
	}
	keys[nkeys].k = k;
	keys[nkeys].mod = mod;
	nkeys++;
}

static void
loadkeys(char *file)
{
	FILE *f;
	int c;

	f = fopen(file, "r");
	if(!f)
		die(file);
	while((c = fgetc(f)) != EOF){
		if(c == '\n' || c == '\r')
			continue;
		if(c != '^'){
			addkey(c, 0);
			continue;
		}
		c = fgetc(f);
		if(c == EOF)
			break;
		switch(c){
		case 'B':
			addkey(Kback, 0);
			break;
		case 'R':
			addkey(Kret, 0);
			break;
		case 'E':
			addkey(Kesc, 0);
			break;
		default:
			addkey(c, Mctrl);
			break;
		}
	}
	fclose(f);
}

static void
dial(void)
{
	struct sockaddr_un addr;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(fd < 0)
		die("socket");
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path),
		"/tmp/strans.%d", getuid());
	if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
		die("connect");
}

static void
sendkey(int key, int mod)
{
	unsigned char req[4];

	req[0] = 0;
	req[1] = mod;
	req[2] = key & 0xff;
	req[3] = (key >> 8) & 0xff;
	if(write(fd, req, 4) != 4)
		die("write");
}

static int
readresp(void)
{
	unsigned char buf[256];
	int n;

	if(read(fd, buf, 2) != 2)
		return -1;
	n = buf[1];
	if(n > 0 && read(fd, buf + 2, n) != n)
		return -1;
	return 0;
}

static double
now(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int
main(int argc, char **argv)
{
	int i, j, niter;
	double t0, t1, dt;

	if(argc < 2){
		fprintf(stderr, "usage: bench file [niter]\n");
		exit(1);
	}

	loadkeys(argv[1]);
	niter = argc > 2 ? atoi(argv[2]) : 1000;
	dial();
	t0 = now();
	for(i = 0; i < niter; i++)
		for(j = 0; j < nkeys; j++){
			sendkey(keys[j].k, keys[j].mod);
			if(readresp() < 0){
				fprintf(stderr, "failed at iter %d key %d\n", i, j);
				exit(1);
			}
		}
	t1 = now();
	dt = t1 - t0;
	printf("%d iters x %d keys = %d keys\n", niter, nkeys, niter * nkeys);
	printf("%.3f ms total, %.3f us/key, %.3f us/iter\n",
		dt * 1000, dt * 1e6 / (niter * nkeys), dt * 1e6 / niter);
	close(fd);
	return 0;
}
