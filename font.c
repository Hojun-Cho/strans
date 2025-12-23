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

static stbtt_fontinfo font;
static uchar *fontdata;
static float scale;
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

void
fontinit(char *path)
{
	int fd, a;
	long sz, n;

	fd = open(path, OREAD);
	if(fd < 0)
		die("can't open font: %s", path);
	sz = seek(fd, 0, 2);
	seek(fd, 0, 0);
	fontdata = emalloc(sz);
	n = readn(fd, fontdata, sz);
	close(fd);
	if(n != sz)
		die("can't read font: %s", path);
	if(!stbtt_InitFont(&font, fontdata, stbtt_GetFontOffsetForIndex(fontdata, 0)))
		die("can't init font: %s", path);
	scale = stbtt_ScaleForPixelHeight(&font, Fontsz);
	for(a = 0; a < 256; a++){
		blendtab[0][a] = blend(Colbg, a);
		blendtab[1][a] = blend(Colsel, a);
	}
}

void
putfont(u32int *buf, int w, int h, int px, int py, Rune r)
{
	Glyph *g;
	int i, j, x, y, a, sel;
	u32int *p;

	if(r >= Nglyphs)
		return;
	g = &cache[r];
	if(g->bmp == nil){
		g->bmp = stbtt_GetCodepointBitmap(&font, scale, scale, r, &g->w, &g->h, &g->ox, &g->oy);
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
