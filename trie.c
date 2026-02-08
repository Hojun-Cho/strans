#include "dat.h"
#include "fn.h"

static int
newnode(Trie *t)
{
	int i;

	if(t->n >= t->cap){
		t->cap *= 2;
		t->nodes = erealloc(t->nodes, t->cap * sizeof(Tnode));
	}
	i = t->n++;
	memset(&t->nodes[i], 0, sizeof(Tnode));
	t->nodes[i].child = -1;
	t->nodes[i].sibling = -1;
	return i;
}

static int
find(Trie *t, int ni, char c)
{
	int pi;

	for(pi = t->nodes[ni].child; pi >= 0; pi = t->nodes[pi].sibling)
		if(t->nodes[pi].c == c)
			return pi;
	return -1;
}

static int
add(Trie *t, int ni, char c)
{
	int pi;

	pi = newnode(t);
	t->nodes[pi].c = c;
	t->nodes[pi].sibling = t->nodes[ni].child;
	t->nodes[ni].child = pi;
	return pi;
}

static void
insert(Trie *t, char *key, int klen, char *val, int vlen)
{
	int ni, ci;
	int i;

	ni = t->root;
	for(i = 0; i < klen; i++){
		ci = find(t, ni, key[i]);
		if(ci < 0)
			ci = add(t, ni, key[i]);
		ni = ci;
	}
	t->nodes[ni].val = val;
	t->nodes[ni].vlen = vlen;
}

Trie*
trieopen(char *path)
{
	Trie *t;
	Biobuf *b;
	char *line, *tab, *key, *val;
	int klen, vlen;

	b = Bopen(path, OREAD);
	if(b == nil)
		die("can't open: %s", path);
	t = emalloc(sizeof(*t));
	t->cap = 1024;
	t->nodes = emalloc(t->cap * sizeof(Tnode));
	t->n = 0;
	t->root = newnode(t);
	while((line = Brdstr(b, '\n', 1)) != nil){
		if(line[0] == '\0'){
			free(line);
			continue;
		}
		tab = strchr(line, '\t');
		if(tab == nil || tab[1] == '\0')
			die("malformed map: %s", path);
		*tab = '\0';
		key = line;
		klen = tab - line;
		val = tab + 1;
		vlen = strlen(val);
		insert(t, key, klen, val, vlen);
	}
	Bterm(b);
	return t;
}

char*
trieget(Trie *t, char *key, int klen, int *vlen)
{
	int ni;
	int i;

	ni = t->root;
	for(i = 0; i < klen; i++){
		ni = find(t, ni, key[i]);
		if(ni < 0)
			return nil;
	}
	if(t->nodes[ni].val == nil)
		return nil;
	*vlen = t->nodes[ni].vlen;
	return t->nodes[ni].val;
}

int
trielookup(Trie *t, char *key, int klen, char **val, int *vlen)
{
	int ni;
	int i;

	ni = t->root;
	for(i = 0; i < klen; i++){
		ni = find(t, ni, key[i]);
		if(ni < 0)
			return 0;
	}
	if(val != nil && t->nodes[ni].val != nil){
		*val = t->nodes[ni].val;
		*vlen = t->nodes[ni].vlen;
	}
	return 1;
}
