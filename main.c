#include "dat.h"
#include "fn.h"

Channel *drawc;
Channel *keyc;
Channel *dictreqc;
Channel *dictresc;
char *fontpath;

int
threadmaybackground(void)
{
	return 1;
}

void
usage(void)
{
	fprint(2, "usage: strans mapdir fontpath\n");
	threadexitsall("usage");
}

void
die(char *fmt, ...)
{
	va_list ap;

	fprint(2, "strans: ");
	va_start(ap, fmt);
	vfprint(2, fmt, ap);
	va_end(ap);
	fprint(2, "\n");
	threadexitsall("die");
}

void*
emalloc(ulong n)
{
	void *p;

	p = malloc(n);
	if(p == nil)
		die("out of memory");
	memset(p, 0, n);
	return p;
}

void*
erealloc(void *p, ulong n)
{
	p = realloc(p, n);
	if(p == nil)
		die("out of memory");
	return p;
}

void
threadmain(int argc, char **argv)
{
	if(argc != 3)
		usage();

	fontpath = argv[2];
	drawc = chancreate(sizeof(Drawcmd), 0);
	keyc = chancreate(sizeof(Keyreq), 0);
	dictreqc = chancreate(sizeof(Dictreq), 4);
	dictresc = chancreate(sizeof(Dictres), 0);
	mapinit(argv[1]);
	dictinit(argv[1]);
	proccreate(drawthread, nil, 16384);
	proccreate(srvthread, nil, 16384);
	threadcreate(dictthread, nil, 16384);
	threadcreate(imthread, nil, 16384);

	threadexits(nil);
}
