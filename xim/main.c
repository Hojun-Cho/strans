#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <xcb/xcb.h>
#include "imdkit.h"
#include "encoding.h"
#include "ipc.h"

static xcb_connection_t *conn;
static xcb_im_t *xim;
static int srvfd;
static xcb_keysym_t *kmap;
static uint8_t minkc, maxkc;
static uint8_t symsper;

static char *encs[] = {"COMPOUND_TEXT", "en_US.UTF-8", ""};
static uint32_t styles[] = {
	XCB_IM_PreeditNothing | XCB_IM_StatusNothing,
	XCB_IM_PreeditNone | XCB_IM_StatusNone,
};

static void
die(char *msg)
{
	fprintf(stderr, "strans-xim: %s\n", msg);
	exit(1);
}

static void
kinit(void)
{
	xcb_get_keyboard_mapping_cookie_t c;
	xcb_get_keyboard_mapping_reply_t *r;
	const xcb_setup_t *setup;
	xcb_keysym_t *syms;
	int i, n;

	setup = xcb_get_setup(conn);
	if(setup == NULL)
		die("xcb_get_setup failed");
	minkc = setup->min_keycode;
	maxkc = setup->max_keycode;
	c = xcb_get_keyboard_mapping(conn, minkc, maxkc - minkc + 1);
	r = xcb_get_keyboard_mapping_reply(conn, c, NULL);
	if(r == NULL)
		die("keyboard mapping failed");
	symsper = r->keysyms_per_keycode;
	n = xcb_get_keyboard_mapping_keysyms_length(r);
	kmap = malloc(n * sizeof(xcb_keysym_t));
	if(kmap == NULL)
		die("malloc failed");
	syms = xcb_get_keyboard_mapping_keysyms(r);
	for(i = 0; i < n; i++)
		kmap[i] = syms[i];
	free(r);
}

static uint32_t
kget(uint8_t kc, uint16_t state)
{
	int col;

	if(kmap == NULL || kc < minkc || kc > maxkc)
		return 0;
	col = (state & Mshift) ? 1 : 0;
	if(col >= symsper)
		col = 0;
	return kmap[(kc - minkc) * symsper + col];
}

static xcb_screen_t*
getscreen(int scr)
{
	xcb_screen_iterator_t iter;
	const xcb_setup_t *setup;

	setup = xcb_get_setup(conn);
	if(setup == NULL)
		die("xcb_get_setup failed");
	iter = xcb_setup_roots_iterator(setup);
	for(; iter.rem; scr--, xcb_screen_next(&iter))
		if(scr == 0)
			return iter.data;
	die("no screen");
	return NULL;
}

static void
commit(xcb_im_input_context_t *ic, char *s, int len)
{
	char *ct;
	size_t clen;

	if(len == 0)
		return;
	ct = xcb_utf8_to_compound_text(s, len, &clen);
	if(ct == NULL)
		return;
	xcb_im_commit_string(xim, ic, XCB_XIM_LOOKUP_CHARS, ct, clen, 0);
	xcb_flush(conn);
	free(ct);
}

static void
srvconnect(void)
{
	struct sockaddr_un addr;

	srvfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(srvfd < 0)
		die("socket failed");
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path), "/tmp/strans.%d", getuid());
	if(connect(srvfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
		die("can't connect to strans");
}

static int
readresp(xcb_im_input_context_t *ic)
{
	unsigned char buf[64];
	int n;

	if(read(srvfd, buf, 2) != 2)
		return -1;
	n = buf[1];
	if(n > 0 && (size_t)n < sizeof(buf) && read(srvfd, buf+2, n) == n)
		commit(ic, (char*)(buf+2), n);
	return buf[0];
}

static void
kpress(xcb_im_input_context_t *ic, xcb_key_press_event_t *ev)
{
	unsigned char buf[4];
	uint32_t key;

	key = kget(ev->detail, ev->state);
	if(key >= 0xff00)
		key = Kspec + (key - 0xff00);
	buf[0] = 0;
	buf[1] = ev->state;
	buf[2] = key;
	buf[3] = key >> 8;
	if(write(srvfd, buf, 4) != 4)
		die("write failed");
	if(readresp(ic) == 0)
		xcb_im_forward_event(xim, ic, ev);
	xcb_flush(conn);
}

static void
reset(xcb_im_input_context_t *ic)
{
	unsigned char buf[4];
	
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = Kesc & 0xff;
	buf[3] = Kesc >> 8;
	if(write(srvfd, buf, 4) != 4)
		die("write failed");
	readresp(ic);
}

static void
callback(xcb_im_t *im, xcb_im_client_t *client, xcb_im_input_context_t *ic,
	const xcb_im_packet_header_fr_t *hdr, void *frame, void *arg, void *user)
{
	xcb_key_press_event_t *ev;

	switch(hdr->major_opcode){
	case XCB_XIM_FORWARD_EVENT:
		ev = arg;
		if(ev != NULL && (ev->response_type & ~0x80) == XCB_KEY_PRESS)
			kpress(ic, ev);
		break;
	case XCB_XIM_UNSET_IC_FOCUS:
		reset(ic);
		break;
	}
}

static void
ximinit(void)
{
	xcb_screen_t *screen;
	xcb_window_t win;
	xcb_im_styles_t st;
	xcb_im_encodings_t enc;
	int scr;

	st.nStyles = 2;
	st.styles = styles;
	enc.nEncodings = 3;
	enc.encodings = encs;
	xcb_compound_text_init();
	conn = xcb_connect(NULL, &scr);
	if(conn == NULL || xcb_connection_has_error(conn))
		die("xcb_connect failed");
	screen = getscreen(scr);
	kinit();
	win = xcb_generate_id(conn);
	xcb_create_window(conn, XCB_COPY_FROM_PARENT, win, screen->root,
		0, 0, 1, 1, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
		screen->root_visual, 0, NULL);
	xim = xcb_im_create(conn, scr, win, "strans",
		XCB_IM_ALL_LOCALES, &st, NULL, NULL, &enc,
		XCB_EVENT_MASK_KEY_PRESS, callback, NULL);
	if(xim == NULL || !xcb_im_open_im(xim))
		die("xcb_im failed");
}

int
main(void)
{
	xcb_generic_event_t *ev;

	srvconnect();
	ximinit();
	for(;;){
		ev = xcb_wait_for_event(conn);
		if(ev == NULL)
			break;
		xcb_im_filter_event(xim, ev);
		free(ev);
	}
	return 0;
}
