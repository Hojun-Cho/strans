#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "dat.h"
#include "fn.h"

typedef struct Glyph Glyph;
struct Glyph
{
	uchar *bmp;
	int w, h, ox, oy;
};

enum { Maxfonts = 4 };

static stbtt_fontinfo fonts[Maxfonts];
static uchar *fontdata[Maxfonts];
static float scale[Maxfonts];
static int nfonts;
static Glyph cache[Nglyphs];
static u32int blendtab[2][256];

static u32int
blend(u32int bg, int a)
{
	int r, g, b, inv;

	inv = 255 - a;
	r = (bg >> 16 & 0xff) * inv / 255;
	g = (bg >> 8 & 0xff) * inv / 255;
	b = (bg & 0xff) * inv / 255;
	return (r << 16) | (g << 8) | b;
}

static void
loadfont(char *path)
{
	int fd;
	long sz, n;

	if(nfonts >= Maxfonts)
		die("too many fonts");
	fd = open(path, OREAD);
	if(fd < 0)
		die("can't open font: %s", path);
	sz = seek(fd, 0, 2);
	seek(fd, 0, 0);
	fontdata[nfonts] = emalloc(sz);
	n = readn(fd, fontdata[nfonts], sz);
	close(fd);
	if(n != sz)
		die("can't read font: %s", path);
	if(!stbtt_InitFont(&fonts[nfonts], fontdata[nfonts], stbtt_GetFontOffsetForIndex(fontdata[nfonts], 0)))
		die("can't init font: %s", path);
	scale[nfonts] = stbtt_ScaleForPixelHeight(&fonts[nfonts], Fontsz);
	nfonts++;
}

static int
isfont(char *name)
{
	char *p;

	p = strrchr(name, '.');
	if(p == nil)
		return 0;
	return strcmp(p, ".ttf") == 0 || strcmp(p, ".otf") == 0;
}

void
fontinit(char *dir)
{
	int fd, n, i, a;
	Dir *d;
	char path[256];

	fd = open(dir, OREAD);
	if(fd < 0)
		die("can't open font dir: %s", dir);
	n = dirreadall(fd, &d);
	close(fd);
	if(n < 0)
		die("can't read font dir: %s", dir);
	for(i = 0; i < n; i++){
		if(isfont(d[i].name)){
			snprint(path, sizeof path, "%s/%s", dir, d[i].name);
			loadfont(path);
		}
	}
	free(d);
	if(nfonts == 0)
		die("no fonts in %s", dir);
	for(a = 0; a < 256; a++){
		blendtab[0][a] = blend(Colbg, a);
		blendtab[1][a] = blend(Colsel, a);
	}
}

void
putfont(u32int *buf, int w, int h, int px, int py, Rune r)
{
	Glyph *g;
	int i, j, x, y, a, sel, f;
	u32int *p;

	if(r >= Nglyphs)
		return;
	g = &cache[r];
	if(g->bmp == nil){
		for(f = 0; f < nfonts; f++){
			if(stbtt_FindGlyphIndex(&fonts[f], r) == 0)
				continue;
			g->bmp = stbtt_GetCodepointBitmap(&fonts[f], scale[f], scale[f], r, &g->w, &g->h, &g->ox, &g->oy);
			if(g->bmp != nil)
				break;
		}
		if(g->bmp == nil)
			return;
	}
	for(j = 0; j < g->h; j++){
		y = py + j + g->oy + Fontsz - Fontbase;
		if(y < 0 || y >= h)
			continue;
		for(i = 0; i < g->w; i++){
			x = px + i + g->ox;
			if(x < 0 || x >= w)
				continue;
			a = g->bmp[j * g->w + i];
			if(a > 0){
				p = &buf[y * w + x];
				sel = (*p == Colsel) ? 1 : 0;
				*p = blendtab[sel][a];
			}
		}
	}
}
