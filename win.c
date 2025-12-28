#include <xcb/xcb.h>
#include "dat.h"
#include "fn.h"

enum {
	Asciitofull = 0xFEE0,
};

extern char *fontdir;

static xcb_connection_t *conn;
static xcb_screen_t *scr;
static xcb_window_t win;
static xcb_gcontext_t gc;
static u32int *img;
static int depth;

static xcb_screen_t*
getscr(xcb_connection_t *c, int n)
{
	xcb_screen_iterator_t i;

	for(i = xcb_setup_roots_iterator(xcb_get_setup(c)); i.rem; xcb_screen_next(&i))
		if(n-- == 0)
			return i.data;
	return nil;
}

static void
wininit(void)
{
	int n;
	u32int mask, vals[4];

	conn = xcb_connect(nil, &n);
	if(conn == nil || xcb_connection_has_error(conn))
		die("xcb_connect");
	scr = getscr(conn, n);
	if(scr == nil)
		die("no screen");
	depth = scr->root_depth;
	win = xcb_generate_id(conn);
	mask = XCB_CW_BACK_PIXEL | XCB_CW_BORDER_PIXEL |
	       XCB_CW_OVERRIDE_REDIRECT | XCB_CW_SAVE_UNDER;
	vals[0] = Colbg;
	vals[1] = 0;
	vals[2] = 1;
	vals[3] = 1;
	xcb_create_window(conn, XCB_COPY_FROM_PARENT, win, scr->root,
		0, 0, 1, 1, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
		scr->root_visual, mask, vals);
	gc = xcb_generate_id(conn);
	xcb_create_gc(conn, gc, win, 0, nil);
	img = emalloc(Imgw * Imgh * sizeof(img[0]));
	fontinit(fontdir);
}

static void
drawstr(u32int *buf, int x, int y, Rune *r, int n, int maxw, int maxh)
{
	while(n-- > 0){
		putfont(buf, maxw, maxh, x, y, *r++);
		x += Fontsz;
	}
}

static void
drawkouho(Drawcmd *dc, int first, int n, int w, int h)
{
	int sely, y, i;
	Str *s;

	memset(img, (uchar)Colbg, w * h * sizeof(u32int));
	drawstr(img, 0, 0, dc->preedit.r, dc->preedit.n, w, h);
	sely = Fontsz + (dc->sel - first) * Fontsz;
	memset(img + sely * w, (uchar)Colsel, Fontsz * w * sizeof(u32int));
	for(i = 0, y = Fontsz; i < n; i++, y += Fontsz){
		s = &dc->kouho[first+i];
		putfont(img, w, h, 0, y, '1' + i + Asciitofull);
		drawstr(img, 2*Fontsz, y, s->r, s->n, w, h);
	}
}

static void
putimage(int w, int h)
{
	xcb_put_image(conn, XCB_IMAGE_FORMAT_Z_PIXMAP, win, gc,
		w, h, 0, 0, 0, depth, w * h * 4, (u8int*)img);
	xcb_flush(conn);
}

static void
winhide(void)
{
	xcb_unmap_window(conn, win);
	xcb_flush(conn);
}

static void
winshow(Drawcmd *dc)
{
	int px, py, w, h, i, n, first, maxw;
	u32int vals[4];
	xcb_query_pointer_reply_t *ptr;
	xcb_query_pointer_cookie_t cookie;

	cookie = xcb_query_pointer(conn, scr->root);
	first = dc->sel >= Maxdisp ? dc->sel - Maxdisp + 1 : 0;
	n = min(dc->nkouho - first, Maxdisp);
	maxw = dc->preedit.n;
	for(i = 0; i < n; i++)
		maxw = max(maxw, dc->kouho[first+i].n);
	ptr = xcb_query_pointer_reply(conn, cookie, nil);
	if(ptr == nil)
		die("xcb_query_pointer");
	px = ptr->root_x + 10;
	py = ptr->root_y + 10;
	free(ptr);
	vals[3] = h = (n + 1) * Fontsz;
	vals[2] = w = (maxw + 3) * Fontsz;
	vals[1] = py = max(0, min(py, scr->height_in_pixels - h));
	vals[0] = px = max(0, min(px, scr->width_in_pixels - w));
	xcb_configure_window(conn, win,
		XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
		XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
		vals);
	xcb_map_window(conn, win);
	drawkouho(dc, first, n, w, h);
	putimage(w, h);
}

void
drawthread(void*)
{
	Drawcmd dc;

	threadsetname("draw");
	wininit();
	while(chanrecv(drawc, &dc) > 0){
		if(dc.nkouho == 0 && dc.preedit.n == 0)
			winhide();
		else
			winshow(&dc);
	}
}
