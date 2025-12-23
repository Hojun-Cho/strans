#include "dat.h"
#include "fn.h"

enum
{
	Maxrunes = nelem(((Str*)0)->r),
};

void
sinit(Str *s, char *src, int n)
{
	int len;

	s->n = 0;
	while(n > 0 && s->n < Maxrunes){
		len = chartorune(&s->r[s->n], src);
		s->n++;
		src += len;
		n -= len;
	}
}

void
sclear(Str *s)
{
	s->n = 0;
}

void
sputr(Str *s, Rune r)
{
	if(s->n >= Maxrunes)
		die("sputr overflow");
	s->r[s->n++] = r;
}

void
spopr(Str *s)
{
	if(s->n > 0)
		s->r[--s->n] = 0;
}

void
sappend(Str *dst, Str *src)
{
	int i;

	for(i = 0; i < src->n && dst->n < Maxrunes; i++)
		dst->r[dst->n++] = src->r[i];
}

int
scmp(Str *a, Str *b)
{
	int i;

	if(a->n != b->n)
		return 1;
	for(i = 0; i < a->n; i++)
		if(a->r[i] != b->r[i])
			return 1;
	return 0;
}

int
stoutf(Str *s, char *buf, int sz)
{
	int i, n, len;

	n = 0;
	for(i = 0; i < s->n && n < sz - UTFmax; i++){
		len = runetochar(buf + n, &s->r[i]);
		n += len;
	}
	buf[n] = '\0';
	return n;
}

Rune
slastr(Str *s)
{
	if(s->n < 1)
		return 0;
	return s->r[s->n-1];
}
