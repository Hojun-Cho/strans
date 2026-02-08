#include "dat.h"
#include "fn.h"

static Im im;

Lang langs[] = {
	{LangEN,  "english", nil, nil, nil},
	{LangJP,  "hira",   "kanji",  nil, nil},
	{LangJPK, "kata",   "kanji",  nil, nil},
	{LangKO,  "hangul", nil, nil, nil},
	{LangEMOJI, "emoji", "emoji", nil, nil},
	{LangVI, "telex", nil, nil, nil},
};
int nlang = nelem(langs);

static void
clearkouho(void)
{
	im.nkouho = 0;
	im.sel = 0;
}

static void
setkouho(Dictres *res)
{
	int i;

	clearkouho();
	for(i = 0; i < res->nkouho; i++){
		im.kouho[i] = res->kouho[i];
		im.nkouho++;
	}
}

static void
show(void)
{
	Drawcmd dc;
	int i;

	sclear(&dc.preedit);
	if(!mapget(im.l->map, &im.pre, &dc.preedit))
		dc.preedit = im.pre;
	dc.nkouho = im.nkouho;
	dc.sel = im.sel;
	for(i = 0; i < dc.nkouho; i++)
		dc.kouho[i] = im.kouho[i];
	chansend(drawc, &dc);
}

static void
reset(void)
{
	sclear(&im.pre);
	clearkouho();
	show();
}

static void
dictq(void)
{
	Str dict;
	Dictreq req;

	if(!mapget(im.l->map, &im.pre, &dict)){
		clearkouho();
		show();
		return;
	}
	req.key = dict;
	req.lang = im.l->lang;
	req.pre = im.pre;
	channbsend(dictreqc, &req);
}

static int
checklang(int c)
{
	Lang *l;

	l = getlang(c);
	if(l == nil)
		return 0;
	im.l = l;
	return 1;
}

static void
commit(Str *com)
{
	Str kana;

	if(mapget(im.l->map, &im.pre, &kana))
		sappend(com, &kana);
	else
		sappend(com, &im.pre);
	sclear(&im.pre);
}

static int
dotrans(Rune c, Str *com)
{
	Emit e;
	Dictreq req;

	e = transmap(&im, c);
	if(e.s.n > 0)
		sappend(com, &e.s);
	sclear(&im.pre);
	sappend(&im.pre, &e.next);
	if(e.eat && e.dict.n > 0){
		req.key = e.dict;
		req.lang = im.l->lang;
		req.pre = im.pre;
		channbsend(dictreqc, &req);
	}
	return e.eat;
}

Lang*
getlang(int lang)
{
	int i;

	for(i = 0; i < nelem(langs); i++)
		if(langs[i].lang == lang)
			return &langs[i];
	return nil;
}

static int
maplookup(Trie *t, Str *key, Str *out)
{
	char buf[256], *v;
	int klen, vlen;

	if(key->n == 0)
		return 0;
	v = nil;
	vlen = 0;
	klen = stoutf(key, buf, sizeof(buf));
	if(!trielookup(t, buf, klen, &v, &vlen))
		return 0;
	if(out != nil && v != nil)
		sinit(out, v, vlen);
	return 1;
}

Emit
transmap(Im *im, Rune c)
{
	Emit e = {0};
	Str key;
	Trie *t;

	t = im->l->map;
	key = im->pre;
	sputr(&key, c);
	if(maplookup(t, &key, &e.dict)){
		e.eat = 1;
		e.next = key;
		return e;
	}
	if(!mapget(t, &im->pre, &e.s))
		e.s = im->pre;
	sclear(&key);
	sputr(&key, c);
	if(!maplookup(t, &key, &e.dict)){
		sputr(&e.s, c);
		return e;
	}
	e.eat = 1;
	sputr(&e.next, c);
	return e;
}

static int
keystroke(u32int ks, u32int mod, Str *com)
{
	int n, off;

	if(ks == Ksuper || ks == Kshift)
		return 1;
	if(ks == Kdown || ks == Kup){
		if(im.nkouho == 0)
			return 0;
		if(ks == Kdown && im.sel < im.nkouho - 1)
			im.sel++;
		if(ks == Kup && im.sel > 0)
			im.sel--;
		show();
		return 1;
	}
	n = ks - '1';
	off = 0;
	if(im.sel >= Maxdisp)
		off = im.sel - Maxdisp + 1;
	if(n >= 0 && n < Maxdisp && off + n < im.nkouho){
		sappend(com, &im.kouho[off + n]);
		reset();
		return 1;
	}
	if(ks == Ktab || ks == Kret){
		if(im.sel >= 0 && im.sel < im.nkouho){
			sappend(com, &im.kouho[im.sel]);
			reset();
			return 1;
		}
		if(im.pre.n > 0){
			commit(com);
			reset();
			return 1;
		}
		return 0;
	}
	if(ks == Kback){
		if(im.pre.n == 0)
			return 0;
		spopr(&im.pre);
		if(im.pre.n == 0){
			reset();
			return 1;
		}
		dictq();
		return 1;
	}
	if(ks == Kesc){
		if(im.pre.n == 0)
			return 0;
		reset();
		return 1;
	}
	if(ks >= Kspec || (mod & (Malt|Msuper))){
		commit(com);
		reset();
		return 0;
	}
	if(mod & Mctrl){
		reset();
		if(ks >= 'a' && ks <= 'z')
			ks -= 'a' - 1;
		if(checklang(ks))
			return 1;
		return 0;
	}
	if(ks > 0x7f || ks == ' '){
		commit(com);
		sputr(com, ks);
		reset();
		return 1;
	}
	dotrans(ks, com);
	show();
	return 1;
}

static void
init(void)
{
	memset(&im, 0, sizeof(im));
	im.l = getlang(LangEN);
}

void
imthread(void*)
{
	Keyreq kr;
	Dictres res;
	Str com;
	uchar out[256];
	int n, len, r;
	Alt alts[] = {
		{keyc, &kr, CHANRCV, nil},
		{dictresc, &res, CHANRCV, nil},
		{nil, nil, CHANEND, nil},
	};

	threadsetname("im");
	init();
	for(;;){
		switch(alt(alts)){
		case 0:
			sclear(&com);
			r = keystroke(kr.ks, kr.mod, &com);
			out[0] = r;
			out[1] = 0;
			n = 2;
			if(com.n > 0){
				len = stoutf(&com, (char*)(out+2), sizeof(out)-2);
				out[1] = len;
				n += len;
			}
			write(kr.fd, out, n);
			break;
		case 1:
			if(scmp(&res.key, &im.pre) == 0){
				setkouho(&res);
				show();
			}
			break;
		}
	}
}

int
mapget(Trie *t, Str *key, Str *out)
{
	char buf[256], *v;
	int klen, vlen;

	if(key->n == 0)
		return 0;
	klen = stoutf(key, buf, sizeof(buf));
	v = trieget(t, buf, klen, &vlen);
	if(v == nil)
		return 0;
	sinit(out, v, vlen);
	return 1;
}

void
mapinit(char *dir)
{
	char path[1024];
	int i;

	for(i = 0; i < nelem(langs); i++){
		if(langs[i].mapname == nil)
			continue;
		snprint(path, sizeof(path), "%s/%s.map", dir, langs[i].mapname);
		langs[i].map = trieopen(path);
	}
}

