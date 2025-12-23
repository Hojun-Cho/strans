#include "dat.h"
#include "fn.h"

static void
dictlkup(Dictreq *req, Dictres *res)
{
	Lang *l;
	Hmap *dict;
	Hnode *n;
	char *p, *e, *sp;
	Str tmp;

	res->nkouho = 0;
	if(req->key.n == 0)
		return;
	res->kouho[0] = req->key;
	res->nkouho = 1;
	l = getlang(req->lang);
	dict = l ? l->dict : nil;
	if(dict == nil)
		return;
	n = hmapget(dict, &req->key);
	if(n == nil || n->kanalen == 0)
		return;
	p = n->kana;
	e = p + n->kanalen;
	while(res->nkouho < Maxkouho && p < e){
		sp = p;
		while(p < e && *p != ' ')
			p++;
		sinit(&tmp, sp, p - sp);
		if(scmp(&tmp, &req->key) != 0)
			res->kouho[res->nkouho++] = tmp;
		if(p < e)
			p++;
	}
}

void
dictthread(void*)
{
	Dictreq req;
	Dictres res;

	threadsetname("dict");
	for(;;){
		if(chanrecv(dictreqc, &req) < 0)
			break;
		while(channbrecv(dictreqc, &req) > 0)
			;
		dictlkup(&req, &res);
		res.key = req.line;
		chansend(dictresc, &res);
	}
}

static Hmap*
opendict(char *path)
{
	Hmap *h;
	Biobuf *b;
	Str key, val;
	char *line, *tab;
	int len;

	b = Bopen(path, OREAD);
	if(b == nil)
		die("can't open: %s", path);
	h = hmapalloc(4096, 0);
	while((line = Brdstr(b, '\n', 1)) != nil){
		len = strlen(line);
		if(len == 0 || line[0] == ';'){
			free(line);
			continue;
		}
		tab = strchr(line, '\t');
		if(tab == nil || tab >= line + len - 1){
			free(line);
			continue;
		}
		*tab = '\0';
		sinit(&key, line, tab - line);
		sinit(&val, tab+1, len - (tab - line) - 1);
		hmapset(&h, &key, &val);
		free(line);
	}
	Bterm(b);
	return h;
}

void
dictinit(char *dir)
{
	char path[1024];
	int i;

	for(i = 0; i < nlang; i++){
		if(langs[i].dictname == nil)
			continue;
		snprint(path, sizeof(path), "%s/%s.dict", dir, langs[i].dictname);
		langs[i].dict = opendict(path);
	}
}

