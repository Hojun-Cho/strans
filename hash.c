#include "dat.h"
#include "fn.h"

enum {
	Tagsize = sizeof(Hnode),
};

static uvlong
hash(Str *s)
{
	uvlong h;
	int i;

	h = 7;
	for(i = 0; i < s->n; i++)
		h = h*31 + s->r[i];
	return h;
}

Hmap*
hmapalloc(int nbuckets, int size)
{
	void *store;
	Hmap *h;
	int nsz;

	nsz = Tagsize + size;
	store = emalloc(sizeof(*h) + nbuckets * nsz);
	h = store;
	h->nbs = nbuckets;
	h->nsz = nsz;
	h->len = h->cap = nbuckets;
	h->nodes = (uchar*)store + sizeof(*h);
	return h;
}

static int
keycmp(Hnode *n, Str *key)
{
	char buf[256];
	int len;

	len = stoutf(key, buf, sizeof(buf));
	if(n->klen != len)
		return 1;
	return memcmp(n->key, buf, len);
}

Hnode*
hmapget(Hmap *h, Str *key)
{
	Hnode *n;
	uchar *v;

	v = h->nodes + (hash(key) % h->nbs) * h->nsz;
	for(;;){
		n = (Hnode*)v;
		if(n->filled && keycmp(n, key) == 0)
			return n;
		if(n->next == 0)
			break;
		v = h->nodes + n->next * h->nsz;
	}
	return nil;
}

static char*
str2dup(Str *s)
{
	char buf[256];
	char *p;
	int n;

	n = stoutf(s, buf, sizeof(buf));
	p = emalloc(n + 1);
	memmove(p, buf, n);
	p[n] = '\0';
	return p;
}

int
hmapset(Hmap **store, Str *key, Str *kana)
{
	Hnode *n;
	uchar *v;
	Hmap *h;
	int next;
	vlong diff;

	h = *store;
	v = h->nodes + (hash(key) % h->nbs) * h->nsz;
	for(;;){
		n = (Hnode*)v;
		next = n->next;
		if(n->filled == 0)
			goto replace;
		if(keycmp(n, key) == 0)
			goto replace;
		if(next == 0)
			break;
		v = h->nodes + next * h->nsz;
	}
	if(h->cap == h->len){
		diff = v - h->nodes;
		h->cap *= 2;
		*store = erealloc(*store, sizeof(*h) + h->cap * h->nsz);
		h = *store;
		h->nodes = (uchar*)*store + sizeof(*h);
		v = h->nodes + diff;
		n = (Hnode*)v;
	}
	n->next = h->len;
	memset(h->nodes + h->len * h->nsz, 0, h->nsz);
	h->len++;
	v = h->nodes + n->next * h->nsz;
	n = (Hnode*)v;
replace:
	if(n->filled == 0){
		n->key = str2dup(key);
		n->klen = strlen(n->key);
		n->filled = 1;
	}
	n->next = next;
	if(kana->n > 0){
		n->kana = str2dup(kana);
		n->kanalen = strlen(n->kana);
	}
	return 0;
}
