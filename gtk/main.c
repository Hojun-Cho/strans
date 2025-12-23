#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <gtk/gtk.h>
#include "ipc.h"

typedef struct Im Im;
struct Im
{
	GtkIMContext parent;
	int fd;
};

typedef struct ImClass ImClass;
struct ImClass
{
	GtkIMContextClass parent;
};

static GType imtype;

static void
srvconnect(Im *im)
{
	struct sockaddr_un addr;

	if(im->fd >= 0)
		return;
	im->fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(im->fd < 0)
		return;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path), "/tmp/strans.%d", getuid());
	if(connect(im->fd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
		close(im->fd);
		im->fd = -1;
	}
}

static int
readresp(Im *im, char *buf, int bufsz)
{
	unsigned char hdr[2];
	int n;

	if(read(im->fd, hdr, 2) != 2)
		return -1;
	n = hdr[1];
	buf[0] = '\0';
	if(n > 0 && n < bufsz){
		if(read(im->fd, buf, n) != n)
			return -1;
		buf[n] = '\0';
	}
	return hdr[0];
}

static uint32_t
kget(uint32_t gdk)
{
	if(gdk >= 0xff00)
		return Kspec + (gdk - 0xff00);
	return gdk_keyval_to_unicode(gdk);
}

static uint32_t
mget(uint32_t state)
{
	uint32_t m;

	m = 0;
	if(state & GDK_SHIFT_MASK)
		m |= Mshift;
	if(state & GDK_CONTROL_MASK)
		m |= Mctrl;
	if(state & GDK_MOD1_MASK)
		m |= Malt;
	if(state & GDK_SUPER_MASK)
		m |= Msuper;
	return m;
}

static void
sendreset(Im *im)
{
	unsigned char buf[4];
	char resp[64];

	if(im->fd < 0)
		return;
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = Kesc & 0xff;
	buf[3] = Kesc >> 8;
	if(write(im->fd, buf, 4) == 4)
		readresp(im, resp, sizeof(resp));
}

static gboolean
kpress(GtkIMContext *ctx, GdkEventKey *ev)
{
	Im *im;
	unsigned char buf[4];
	char resp[64];
	uint32_t key, mod;
	int r;

	im = (Im*)ctx;
	if(ev->type != GDK_KEY_PRESS)
		return FALSE;
	srvconnect(im);
	if(im->fd < 0)
		return FALSE;
	key = kget(ev->keyval);
	if(key == 0)
		return FALSE;
	mod = mget(ev->state);
	buf[0] = 0;
	buf[1] = mod;
	buf[2] = key & 0xff;
	buf[3] = key >> 8;
	if(write(im->fd, buf, 4) != 4)
		return FALSE;
	r = readresp(im, resp, sizeof(resp));
	if(r < 0)
		return FALSE;
	if(r != 0 && resp[0] != '\0')
		g_signal_emit_by_name(ctx, "commit", resp);
	return r != 0;
}

static void
reset(GtkIMContext *ctx)
{
	sendreset((Im*)ctx);
}

static void
focusout(GtkIMContext *ctx)
{
	sendreset((Im*)ctx);
}

static void
finalize(GObject *obj)
{
	Im *im;

	im = (Im*)obj;
	if(im->fd >= 0)
		close(im->fd);
	G_OBJECT_CLASS(g_type_class_peek_parent(G_OBJECT_GET_CLASS(obj)))->finalize(obj);
}

static void
init(Im *im)
{
	im->fd = -1;
}

static void
classinit(ImClass *klass)
{
	GtkIMContextClass *ic;
	GObjectClass *oc;

	ic = GTK_IM_CONTEXT_CLASS(klass);
	oc = G_OBJECT_CLASS(klass);
	ic->filter_keypress = kpress;
	ic->reset = reset;
	ic->focus_out = focusout;
	oc->finalize = finalize;
}

static const GtkIMContextInfo info = {
	"strans",
	"strans",
	"strans",
	"",
	"*",
};

static const GtkIMContextInfo *infolist[] = { &info };

G_MODULE_EXPORT void
im_module_init(GTypeModule *mod)
{
	static const GTypeInfo ti = {
		sizeof(ImClass),
		NULL, NULL,
		(GClassInitFunc)classinit,
		NULL, NULL,
		sizeof(Im),
		0,
		(GInstanceInitFunc)init,
	};
	imtype = g_type_module_register_type(mod, GTK_TYPE_IM_CONTEXT, "strans-gtk", &ti, 0);
}

G_MODULE_EXPORT void
im_module_exit(void)
{
}

G_MODULE_EXPORT void
im_module_list(const GtkIMContextInfo ***contexts, int *n)
{
	*contexts = infolist;
	*n = 1;
}

G_MODULE_EXPORT GtkIMContext*
im_module_create(const char *id)
{
	if(strcmp(id, "strans") == 0)
		return g_object_new(imtype, NULL);
	return NULL;
}
