#include "dat.h"
#include "fn.h"

Emit
jptrans(Im *im, Rune c)
{
	Emit e;
	Str key, kana;
	Hmap *h;
	Rune last;
	int hira;

	memset(&e, 0, sizeof(e));
	h = im->l->map;
	hira = (im->l->lang == LangJP);
	key = im->pre;
	sputr(&key, c);
	if(hmapget(h, &key)){
		e.eat = 1;
		e.next = key;
		mapget(h, &key, &e.dict);
		return e;
	}
	last = slastr(&im->pre);
	if(last == 0)
		goto flush;
	key = im->pre;
	key.n--;
	if(mapget(h, &key, &kana)){
		sclear(&key);
		sputr(&key, last);
		sputr(&key, c);
		if(hmapget(h, &key)){
			e.eat = 1;
			e.s = kana;
			sputr(&e.next, last);
			sputr(&e.next, c);
			mapget(h, &e.next, &e.dict);
			return e;
		}
	}
	if(c == 'n' && last == 'n'){
		key = im->pre;
		key.n--;
		if(mapget(h, &key, &kana)){
			e.eat = 1;
			e.s = kana;
			sputr(&e.s, hira ? 0x3093 : 0x30F3);
			sputr(&e.next, c);
			mapget(h, &e.next, &e.dict);
			return e;
		}
	}
	if(c == last && strchr("kgsztdbpmjfchryw", c)){
		key = im->pre;
		key.n--;
		if(mapget(h, &key, &kana)){
			e.eat = 1;
			e.s = kana;
			sputr(&e.s, hira ? 0x3063 : 0x30C3);
			sputr(&e.next, c);
			mapget(h, &e.next, &e.dict);
			return e;
		}
	}

flush:
	mapget(h, &im->pre, &e.s);
	sclear(&key);
	sputr(&key, c);
	if(hmapget(h, &key) == nil){
		e.flush = 1;
		sputr(&e.s, c);
		return e;
	}
	e.eat = 1;
	sputr(&e.next, c);
	mapget(h, &e.next, &e.dict);
	return e;
}
