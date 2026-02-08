#include "dat.h"
#include "fn.h"

enum
{
	Sbase	= 0xAC00,
	Jbase	= 0x3131,
	Jrange	= 51,
	Ncho	= 19,
	Njung	= 21,
	Njong	= 28,
};

static Rune cho[] = {
	L'ㄱ', L'ㄲ', L'ㄴ', L'ㄷ', L'ㄸ',
	L'ㄹ', L'ㅁ', L'ㅂ', L'ㅃ', L'ㅅ',
	L'ㅆ', L'ㅇ', L'ㅈ', L'ㅉ', L'ㅊ',
	L'ㅋ', L'ㅌ', L'ㅍ', L'ㅎ',
};

static Rune jung[] = {
	L'ㅏ', L'ㅐ', L'ㅑ', L'ㅒ', L'ㅓ',
	L'ㅔ', L'ㅕ', L'ㅖ', L'ㅗ', L'ㅘ',
	L'ㅙ', L'ㅚ', L'ㅛ', L'ㅜ', L'ㅝ',
	L'ㅞ', L'ㅟ', L'ㅠ', L'ㅡ', L'ㅢ',
	L'ㅣ',
};

static Rune jong[] = {
	0,      L'ㄱ', L'ㄲ', L'ㄳ', L'ㄴ',
	L'ㄵ', L'ㄶ', L'ㄷ', L'ㄹ', L'ㄺ',
	L'ㄻ', L'ㄼ', L'ㄽ', L'ㄾ', L'ㄿ',
	L'ㅀ', L'ㅁ', L'ㅂ', L'ㅄ', L'ㅅ',
	L'ㅆ', L'ㅇ', L'ㅈ', L'ㅊ', L'ㅋ',
	L'ㅌ', L'ㅍ', L'ㅎ',
};

static int choidx[Jrange] = {
	[L'ㄱ'-Jbase] = 1,  [L'ㄲ'-Jbase] = 2,  [L'ㄴ'-Jbase] = 3,
	[L'ㄷ'-Jbase] = 4,  [L'ㄸ'-Jbase] = 5,  [L'ㄹ'-Jbase] = 6,
	[L'ㅁ'-Jbase] = 7,  [L'ㅂ'-Jbase] = 8,  [L'ㅃ'-Jbase] = 9,
	[L'ㅅ'-Jbase] = 10, [L'ㅆ'-Jbase] = 11, [L'ㅇ'-Jbase] = 12,
	[L'ㅈ'-Jbase] = 13, [L'ㅉ'-Jbase] = 14, [L'ㅊ'-Jbase] = 15,
	[L'ㅋ'-Jbase] = 16, [L'ㅌ'-Jbase] = 17, [L'ㅍ'-Jbase] = 18,
	[L'ㅎ'-Jbase] = 19,
};

static int jungidx[Jrange] = {
	[L'ㅏ'-Jbase] = 1,  [L'ㅐ'-Jbase] = 2,  [L'ㅑ'-Jbase] = 3,
	[L'ㅒ'-Jbase] = 4,  [L'ㅓ'-Jbase] = 5,  [L'ㅔ'-Jbase] = 6,
	[L'ㅕ'-Jbase] = 7,  [L'ㅖ'-Jbase] = 8,  [L'ㅗ'-Jbase] = 9,
	[L'ㅘ'-Jbase] = 10, [L'ㅙ'-Jbase] = 11, [L'ㅚ'-Jbase] = 12,
	[L'ㅛ'-Jbase] = 13, [L'ㅜ'-Jbase] = 14, [L'ㅝ'-Jbase] = 15,
	[L'ㅞ'-Jbase] = 16, [L'ㅟ'-Jbase] = 17, [L'ㅠ'-Jbase] = 18,
	[L'ㅡ'-Jbase] = 19, [L'ㅢ'-Jbase] = 20, [L'ㅣ'-Jbase] = 21,
};

static int jongidx[Jrange] = {
	[L'ㄱ'-Jbase] = 2,  [L'ㄲ'-Jbase] = 3,  [L'ㄳ'-Jbase] = 4,
	[L'ㄴ'-Jbase] = 5,  [L'ㄵ'-Jbase] = 6,  [L'ㄶ'-Jbase] = 7,
	[L'ㄷ'-Jbase] = 8,  [L'ㄹ'-Jbase] = 9,  [L'ㄺ'-Jbase] = 10,
	[L'ㄻ'-Jbase] = 11, [L'ㄼ'-Jbase] = 12, [L'ㄽ'-Jbase] = 13,
	[L'ㄾ'-Jbase] = 14, [L'ㄿ'-Jbase] = 15, [L'ㅀ'-Jbase] = 16,
	[L'ㅁ'-Jbase] = 17, [L'ㅂ'-Jbase] = 18, [L'ㅄ'-Jbase] = 19,
	[L'ㅅ'-Jbase] = 20, [L'ㅆ'-Jbase] = 21, [L'ㅇ'-Jbase] = 22,
	[L'ㅈ'-Jbase] = 23, [L'ㅊ'-Jbase] = 24, [L'ㅋ'-Jbase] = 25,
	[L'ㅌ'-Jbase] = 26, [L'ㅍ'-Jbase] = 27, [L'ㅎ'-Jbase] = 28,
};

#define Choidx(r)	(choidx[(r) - Jbase] - 1)
#define Jungidx(r)	(jungidx[(r) - Jbase] - 1)
#define Jongidx(r)	(jongidx[(r) - Jbase] - 1)

static Rune jamomap[128] = {
	['r'] = L'ㄱ', ['R'] = L'ㄲ',
	['s'] = L'ㄴ', ['S'] = L'ㄴ',
	['e'] = L'ㄷ', ['E'] = L'ㄸ',
	['f'] = L'ㄹ', ['F'] = L'ㄹ',
	['a'] = L'ㅁ', ['A'] = L'ㅁ',
	['q'] = L'ㅂ', ['Q'] = L'ㅃ',
	['t'] = L'ㅅ', ['T'] = L'ㅆ',
	['d'] = L'ㅇ', ['D'] = L'ㅇ',
	['w'] = L'ㅈ', ['W'] = L'ㅉ',
	['c'] = L'ㅊ', ['C'] = L'ㅊ',
	['z'] = L'ㅋ', ['Z'] = L'ㅋ',
	['x'] = L'ㅌ', ['X'] = L'ㅌ',
	['v'] = L'ㅍ', ['V'] = L'ㅍ',
	['g'] = L'ㅎ', ['G'] = L'ㅎ',
	['k'] = L'ㅏ', ['K'] = L'ㅏ',
	['o'] = L'ㅐ', ['O'] = L'ㅒ',
	['i'] = L'ㅑ', ['I'] = L'ㅑ',
	['j'] = L'ㅓ', ['J'] = L'ㅓ',
	['p'] = L'ㅔ', ['P'] = L'ㅖ',
	['u'] = L'ㅕ', ['U'] = L'ㅕ',
	['h'] = L'ㅗ', ['H'] = L'ㅗ',
	['y'] = L'ㅛ', ['Y'] = L'ㅛ',
	['n'] = L'ㅜ', ['N'] = L'ㅜ',
	['b'] = L'ㅠ', ['B'] = L'ㅠ',
	['m'] = L'ㅡ', ['M'] = L'ㅡ',
	['l'] = L'ㅣ', ['L'] = L'ㅣ',
};

typedef struct Cpair Cpair;
struct Cpair
{
	Rune	b;
	Rune	a;
	Rune	r;
};

static Cpair cvow[] = {
	{L'ㅗ', L'ㅏ', L'ㅘ'},
	{L'ㅗ', L'ㅐ', L'ㅙ'},
	{L'ㅗ', L'ㅣ', L'ㅚ'},
	{L'ㅜ', L'ㅓ', L'ㅝ'},
	{L'ㅜ', L'ㅔ', L'ㅞ'},
	{L'ㅜ', L'ㅣ', L'ㅟ'},
	{L'ㅡ', L'ㅣ', L'ㅢ'},
};

static Cpair cjong[] = {
	{L'ㄱ', L'ㅅ', L'ㄳ'},
	{L'ㄴ', L'ㅈ', L'ㄵ'},
	{L'ㄴ', L'ㅎ', L'ㄶ'},
	{L'ㄹ', L'ㄱ', L'ㄺ'},
	{L'ㄹ', L'ㅁ', L'ㄻ'},
	{L'ㄹ', L'ㅂ', L'ㄼ'},
	{L'ㄹ', L'ㅅ', L'ㄽ'},
	{L'ㄹ', L'ㅌ', L'ㄾ'},
	{L'ㄹ', L'ㅍ', L'ㄿ'},
	{L'ㄹ', L'ㅎ', L'ㅀ'},
	{L'ㅂ', L'ㅅ', L'ㅄ'},
};

static Rune
keytojamo(int c)
{
	if(c >= 0 && c < 128)
		return jamomap[c];
	return 0;
}

static Rune
combine(Cpair *t, int n, Rune b, Rune a)
{
	int i;

	for(i = 0; i < n; i++)
		if(t[i].b == b && t[i].a == a)
			return t[i].r;
	return 0;
}

static int
splitpair(Cpair *t, int n, Rune c, Rune *stay, Rune *next)
{
	int i;

	for(i = 0; i < n; i++){
		if(t[i].r == c){
			*stay = t[i].b;
			*next = t[i].a;
			return 1;
		}
	}
	return 0;
}

static Rune
compose(int c, int j, int jo)
{
	return Sbase + c*Njung*Njong + j*Njong + jo;
}

static void
decompose(Rune s, int *c, int *j, int *jo)
{
	int i;

	i = s - Sbase;
	*c = i / (Njung * Njong);
	*j = (i / Njong) % Njung;
	*jo = i % Njong;
}

static int
issyl(Rune r)
{
	return r >= Sbase && r < Sbase + Ncho*Njung*Njong;
}

Emit
transko(Im *im, Rune c)
{
	Emit e;
	Rune jm, jc, last, comb, stay, next;
	int ci, ji, joi, ni, si, vi;

	memset(&e, 0, sizeof e);
	jm = keytojamo(c);
	if(jm == 0){
		if(im->pre.n > 0){
			e.s = im->pre;
			sclear(&im->pre);
		}
		sputr(&e.s, c);
		return e;
	}

	e.eat = 1;
	last = slastr(&im->pre);
	if(last == 0){
		sputr(&e.next, jm);
		return e;
	}

	if(!issyl(last)){
		ci = Choidx(last);
		ji = Jungidx(jm);
		if(ci >= 0 && ji >= 0){
			spopr(&im->pre);
			sputr(&e.next, compose(ci, ji, 0));
			return e;
		}
		if(Jungidx(last) >= 0 && ji >= 0){
			comb = combine(cvow, nelem(cvow), last, jm);
			if(comb){
				spopr(&im->pre);
				sputr(&e.next, comb);
				return e;
			}
		}
		e.s = im->pre;
		sputr(&e.next, jm);
		return e;
	}

	decompose(last, &ci, &ji, &joi);

	if(joi == 0){
		ni = Jongidx(jm);
		if(ni > 0){
			spopr(&im->pre);
			sputr(&e.next, compose(ci, ji, ni));
			return e;
		}
		vi = Jungidx(jm);
		if(vi >= 0){
			comb = combine(cvow, nelem(cvow), jung[ji], jm);
			if(comb){
				ni = Jungidx(comb);
				spopr(&im->pre);
				sputr(&e.next, compose(ci, ni, 0));
				return e;
			}
		}
	}

	if(joi > 0){
		comb = combine(cjong, nelem(cjong), jong[joi], jm);
		if(comb){
			ni = Jongidx(comb);
			if(ni > 0){
				spopr(&im->pre);
				sputr(&e.next, compose(ci, ji, ni));
				return e;
			}
		}
		vi = Jungidx(jm);
		if(vi >= 0){
			jc = jong[joi];
			if(splitpair(cjong, nelem(cjong), jc, &stay, &next)){
				si = Jongidx(stay);
				ni = Choidx(next);
				spopr(&im->pre);
				sputr(&e.s, compose(ci, ji, si));
				sputr(&e.next, compose(ni, vi, 0));
				return e;
			}
			ni = Choidx(jc);
			if(ni >= 0){
				spopr(&im->pre);
				sputr(&e.s, compose(ci, ji, 0));
				sputr(&e.next, compose(ni, vi, 0));
				return e;
			}
		}
	}

	e.s = im->pre;
	sputr(&e.next, jm);
	return e;
}

void
backko(Im *im)
{
	Rune last;
	int c, j, jo;
	Rune stay, next;

	last = slastr(&im->pre);
	if(!issyl(last)){
		if(splitpair(cvow, nelem(cvow), last, &stay, &next)){
			spopr(&im->pre);
			sputr(&im->pre, stay);
			return;
		}
		spopr(&im->pre);
		return;
	}
	decompose(last, &c, &j, &jo);
	spopr(&im->pre);
	if(jo > 0 && splitpair(cjong, nelem(cjong), jong[jo], &stay, &next)){
		sputr(&im->pre, compose(c, j, Jongidx(stay)));
		return;
	}
	if(jo > 0){
		sputr(&im->pre, compose(c, j, 0));
		return;
	}
	if(splitpair(cvow, nelem(cvow), jung[j], &stay, &next)){
		sputr(&im->pre, compose(c, Jungidx(stay), 0));
		return;
	}
	sputr(&im->pre, cho[c]);
}
