#include "dat.h"
#include "fn.h"

static struct {
	Rune	base;
	Rune	tone[5];	/* s f r x j */
} vitone[] = {
	{L'a', {L'á', L'à', L'ả', L'ã', L'ạ'}},
	{L'â', {L'ấ', L'ầ', L'ẩ', L'ẫ', L'ậ'}},
	{L'ă', {L'ắ', L'ằ', L'ẳ', L'ẵ', L'ặ'}},
	{L'e', {L'é', L'è', L'ẻ', L'ẽ', L'ẹ'}},
	{L'ê', {L'ế', L'ề', L'ể', L'ễ', L'ệ'}},
	{L'i', {L'í', L'ì', L'ỉ', L'ĩ', L'ị'}},
	{L'o', {L'ó', L'ò', L'ỏ', L'õ', L'ọ'}},
	{L'ô', {L'ố', L'ồ', L'ổ', L'ỗ', L'ộ'}},
	{L'ơ', {L'ớ', L'ờ', L'ở', L'ỡ', L'ợ'}},
	{L'u', {L'ú', L'ù', L'ủ', L'ũ', L'ụ'}},
	{L'ư', {L'ứ', L'ừ', L'ử', L'ữ', L'ự'}},
	{L'y', {L'ý', L'ỳ', L'ỷ', L'ỹ', L'ỵ'}},
	{L'A', {L'Á', L'À', L'Ả', L'Ã', L'Ạ'}},
	{L'Â', {L'Ấ', L'Ầ', L'Ẩ', L'Ẫ', L'Ậ'}},
	{L'Ă', {L'Ắ', L'Ằ', L'Ẳ', L'Ẵ', L'Ặ'}},
	{L'E', {L'É', L'È', L'Ẻ', L'Ẽ', L'Ẹ'}},
	{L'Ê', {L'Ế', L'Ề', L'Ể', L'Ễ', L'Ệ'}},
	{L'I', {L'Í', L'Ì', L'Ỉ', L'Ĩ', L'Ị'}},
	{L'O', {L'Ó', L'Ò', L'Ỏ', L'Õ', L'Ọ'}},
	{L'Ô', {L'Ố', L'Ồ', L'Ổ', L'Ỗ', L'Ộ'}},
	{L'Ơ', {L'Ớ', L'Ờ', L'Ở', L'Ỡ', L'Ợ'}},
	{L'U', {L'Ú', L'Ù', L'Ủ', L'Ũ', L'Ụ'}},
	{L'Ư', {L'Ứ', L'Ừ', L'Ử', L'Ữ', L'Ự'}},
	{L'Y', {L'Ý', L'Ỳ', L'Ỷ', L'Ỹ', L'Ỵ'}},
};

static int
istone(Rune c)
{
	return c == 's' || c == 'f' || c == 'r' || c == 'x' || c == 'j';
}

static int
toneidx(Rune c)
{
	switch(c){
	case 's': return 0;
	case 'f': return 1;
	case 'r': return 2;
	case 'x': return 3;
	case 'j': return 4;
	}
	return -1;
}

static int
isvowel(Rune c)
{
	int i;

	for(i = 0; i < nelem(vitone); i++)
		if(vitone[i].base == c)
			return 1;
	return 0;
}

static Rune
removetone(Rune c)
{
	int i, j;

	for(i = 0; i < nelem(vitone); i++){
		if(vitone[i].base == c)
			return c;
		for(j = 0; j < 5; j++)
			if(vitone[i].tone[j] == c)
				return vitone[i].base;
	}
	return c;
}

static Rune
applytone(Rune c, int tidx)
{
	int i;
	Rune base;

	base = removetone(c);
	for(i = 0; i < nelem(vitone); i++)
		if(vitone[i].base == base)
			return vitone[i].tone[tidx];
	return c;
}

Emit
transvi(Im *im, Rune c)
{
	Emit e;
	Str mapped, pre;
	int i, tidx, vi, last, penult;
	Rune v, b1, b2;

	if(!istone(c) && c != 'z')
		return transmap(im, c);

	memset(&e, 0, sizeof e);
	if(im->pre.n == 0){
		sputr(&e.s, c);
		return e;
	}
	if(im->pre.r[im->pre.n - 1] == '\\'){
		pre = im->pre;
		pre.n--;
		if(!mapget(im->l->map, &pre, &mapped))
			mapped = pre;
		sputr(&mapped, c);
		e.eat = 1;
		e.s = mapped;
		return e;
	}
	if(!mapget(im->l->map, &im->pre, &mapped))
		mapped = im->pre;
	last = -1;
	penult = -1;
	for(i = 0; i < mapped.n; i++){
		v = removetone(mapped.r[i]);
		if(isvowel(v)){
			penult = last;
			last = i;
		}
	}
	vi = -1;
	if(last >= 0){
		if(last < mapped.n - 1 || penult < 0)
			vi = last;
		else{
			b1 = removetone(mapped.r[penult]);
			b2 = removetone(mapped.r[last]);
			if((b1 == 'o' || b1 == 'O') && (b2 == 'a' || b2 == 'A' || b2 == 'e' || b2 == 'E'))
				vi = last;
			else if((b1 == 'u' || b1 == 'U') && (b2 == 'y' || b2 == 'Y'))
				vi = last;
			else
				vi = penult;
		}
	}
	if(vi < 0){
		e.eat = 1;
		e.s = mapped;
		sputr(&e.s, c);
		return e;
	}

	if(c == 'z')
		mapped.r[vi] = removetone(mapped.r[vi]);
	else{
		tidx = toneidx(c);
		mapped.r[vi] = applytone(mapped.r[vi], tidx);
	}
	e.eat = 1;
	e.next = mapped;
	return e;
}
