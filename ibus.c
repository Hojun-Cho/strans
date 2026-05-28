#include "dat.h"
#include "fn.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <poll.h>
#include <dbus/dbus.h>

enum
{
	Maxwatches	= 32,
	Maxconns	= 16,
	Relmask		= 1<<30,
};

typedef struct Watch Watch;
struct Watch
{
	DBusWatch	*w;
};

static Watch watches[Maxwatches];
static int nwatches;
static DBusConnection *conns[Maxconns];
static int nconns;
static DBusServer *srv;
static char addrfile[256];
static int icctr;
static int busctr;

static DBusHandlerResult onmsg(DBusConnection*, DBusMessage*, void*);

static void
unlinkaddr(void)
{
	if(addrfile[0] != '\0')
		unlink(addrfile);
}

static void
machineid(char *buf, int sz)
{
	int fd, n;

	fd = open("/etc/machine-id", 0);
	if(fd < 0)
		fd = open("/var/lib/dbus/machine-id", 0);
	if(fd < 0){
		buf[0] = '\0';
		return;
	}
	n = read(fd, buf, sz - 1);
	close(fd);
	if(n < 0) n = 0;
	buf[n] = '\0';
	while(n > 0 && (buf[n-1] == '\n' || buf[n-1] == '\r'))
		buf[--n] = '\0';
}

static void
xdisplay(char *host, int hsz, char *num, int nsz)
{
	char *d, *colon, *dot, *p;
	int n;

	strncpy(host, "unix", hsz);
	host[hsz-1] = '\0';
	strncpy(num, "0", nsz);
	num[nsz-1] = '\0';
	d = getenv("DISPLAY");
	if(d == nil || d[0] == '\0')
		return;
	colon = strchr(d, ':');
	if(colon == nil)
		return;
	if(colon > d){
		n = colon - d;
		if(n >= hsz) n = hsz - 1;
		memcpy(host, d, n);
		host[n] = '\0';
	}
	p = colon + 1;
	dot = strchr(p, '.');
	n = dot ? dot - p : (int)strlen(p);
	if(n >= nsz) n = nsz - 1;
	memcpy(num, p, n);
	num[n] = '\0';
}

static int
buildaddrpath(char *buf, int sz)
{
	char mid[64], host[64], num[8], *cfg, *home;
	struct stat st;
	char dir[256];

	machineid(mid, sizeof(mid));
	if(mid[0] == '\0')
		return -1;
	xdisplay(host, sizeof(host), num, sizeof(num));
	cfg = getenv("XDG_CONFIG_HOME");
	if(cfg != nil && cfg[0] != '\0')
		snprintf(dir, sizeof(dir), "%s/ibus/bus", cfg);
	else{
		home = getenv("HOME");
		if(home == nil)
			return -1;
		snprintf(dir, sizeof(dir), "%s/.config/ibus/bus", home);
	}
	if(stat(dir, &st) < 0){
		char tmp[256];
		char *p;
		strncpy(tmp, dir, sizeof(tmp));
		tmp[sizeof(tmp)-1] = '\0';
		for(p = tmp+1; *p; p++)
			if(*p == '/'){
				*p = '\0';
				mkdir(tmp, 0700);
				*p = '/';
			}
		mkdir(tmp, 0700);
	}
	snprintf(buf, sz, "%s/%s-%s-%s", dir, mid, host, num);
	return 0;
}

static int
writeaddr(char *path, char *addr)
{
	FILE *fp;

	fp = fopen(path, "w");
	if(fp == nil)
		return -1;
	fprintf(fp, "IBUS_ADDRESS=%s\n", addr);
	fprintf(fp, "IBUS_DAEMON_PID=%d\n", (int)getpid());
	fclose(fp);
	chmod(path, 0600);
	return 0;
}

static dbus_bool_t
addwatch(DBusWatch *w, void *_)
{
	int i;

	USED(_);
	for(i = 0; i < nwatches; i++)
		if(watches[i].w == nil){
			watches[i].w = w;
			return TRUE;
		}
	if(nwatches >= Maxwatches)
		return FALSE;
	watches[nwatches++].w = w;
	return TRUE;
}

static void
removewatch(DBusWatch *w, void *_)
{
	int i;

	USED(_);
	for(i = 0; i < nwatches; i++)
		if(watches[i].w == w){
			watches[i].w = nil;
			return;
		}
}

static void
togglewatch(DBusWatch *w, void *_)
{
	USED(w);
	USED(_);
}

static u32int
kget(u32int sym)
{
	if(sym >= 0xff00 && sym <= 0xffff)
		return Kspec + (sym - 0xff00);
	return sym;
}

static u32int
mget(u32int state)
{
	u32int m;

	m = 0;
	if(state & (1<<0)) m |= Mshift;
	if(state & (1<<2)) m |= Mctrl;
	if(state & (1<<3)) m |= Malt;
	if(state & (1<<6)) m |= Msuper;
	return m;
}

static int
sendkey(u32int ks, u32int mod, char *out, int sz, int *eaten)
{
	Keyreq kr;
	int p[2];
	uchar hdr[2];
	int n;

	*eaten = 0;
	out[0] = '\0';
	if(pipe(p) < 0)
		return 0;
	kr.fd = p[1];
	kr.ks = ks;
	kr.mod = mod;
	chansend(keyc, &kr);
	if(read(p[0], hdr, 2) != 2){
		close(p[0]);
		close(p[1]);
		return 0;
	}
	*eaten = hdr[0];
	n = hdr[1];
	if(n > 0 && n < sz){
		if(read(p[0], out, n) != n)
			n = 0;
	}else
		n = 0;
	out[n] = '\0';
	close(p[0]);
	close(p[1]);
	return n;
}

static void
sendreset(void)
{
	char buf[64];
	int eaten;

	sendkey(Kesc, 0, buf, sizeof(buf), &eaten);
}

static void
appendibustext(DBusMessageIter *it, const char *s)
{
	DBusMessageIter v, st, attach, alv, ali, attr, alist;
	const char *name = "IBusText";
	const char *aname = "IBusAttrList";

	dbus_message_iter_open_container(it, DBUS_TYPE_VARIANT, "(sa{sv}sv)", &v);
	dbus_message_iter_open_container(&v, DBUS_TYPE_STRUCT, nil, &st);
	dbus_message_iter_append_basic(&st, DBUS_TYPE_STRING, &name);
	dbus_message_iter_open_container(&st, DBUS_TYPE_ARRAY, "{sv}", &attach);
	dbus_message_iter_close_container(&st, &attach);
	dbus_message_iter_append_basic(&st, DBUS_TYPE_STRING, &s);
	dbus_message_iter_open_container(&st, DBUS_TYPE_VARIANT, "(sa{sv}av)", &alv);
	dbus_message_iter_open_container(&alv, DBUS_TYPE_STRUCT, nil, &ali);
	dbus_message_iter_append_basic(&ali, DBUS_TYPE_STRING, &aname);
	dbus_message_iter_open_container(&ali, DBUS_TYPE_ARRAY, "{sv}", &attr);
	dbus_message_iter_close_container(&ali, &attr);
	dbus_message_iter_open_container(&ali, DBUS_TYPE_ARRAY, "v", &alist);
	dbus_message_iter_close_container(&ali, &alist);
	dbus_message_iter_close_container(&alv, &ali);
	dbus_message_iter_close_container(&st, &alv);
	dbus_message_iter_close_container(&v, &st);
	dbus_message_iter_close_container(it, &v);
}

static void
emitcommit(DBusConnection *c, const char *path, const char *text)
{
	DBusMessage *sig;
	DBusMessageIter it;

	sig = dbus_message_new_signal(path, "org.freedesktop.IBus.InputContext", "CommitText");
	if(sig == nil)
		return;
	dbus_message_iter_init_append(sig, &it);
	appendibustext(&it, text);
	dbus_connection_send(c, sig, nil);
	dbus_message_unref(sig);
}

static DBusHandlerResult
handlehello(DBusConnection *c, DBusMessage *m)
{
	DBusMessage *r;
	char name[32];
	const char *np;

	busctr++;
	snprintf(name, sizeof(name), ":1.%d", busctr);
	np = name;
	r = dbus_message_new_method_return(m);
	dbus_message_append_args(r, DBUS_TYPE_STRING, &np, DBUS_TYPE_INVALID);
	dbus_connection_send(c, r, nil);
	dbus_message_unref(r);
	return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult
handlecreate(DBusConnection *c, DBusMessage *m)
{
	DBusMessage *r;
	char path[64];
	const char *pp;

	icctr++;
	snprintf(path, sizeof(path), "/org/freedesktop/IBus/InputContext_%d", icctr);
	pp = path;
	r = dbus_message_new_method_return(m);
	dbus_message_append_args(r, DBUS_TYPE_OBJECT_PATH, &pp, DBUS_TYPE_INVALID);
	dbus_connection_send(c, r, nil);
	dbus_message_unref(r);
	return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult
handlekey(DBusConnection *c, DBusMessage *m)
{
	DBusMessage *r;
	DBusError err;
	dbus_uint32_t sym, code, state;
	char commit[256];
	int eaten, n;
	dbus_bool_t b;
	u32int ks, mod;

	dbus_error_init(&err);
	if(!dbus_message_get_args(m, &err,
		DBUS_TYPE_UINT32, &sym,
		DBUS_TYPE_UINT32, &code,
		DBUS_TYPE_UINT32, &state,
		DBUS_TYPE_INVALID)){
		dbus_error_free(&err);
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}
	USED(code);
	if(state & Relmask){
		r = dbus_message_new_method_return(m);
		b = FALSE;
		dbus_message_append_args(r, DBUS_TYPE_BOOLEAN, &b, DBUS_TYPE_INVALID);
		dbus_connection_send(c, r, nil);
		dbus_message_unref(r);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	ks = kget(sym);
	mod = mget(state);
	n = sendkey(ks, mod, commit, sizeof(commit), &eaten);
	if(n > 0)
		emitcommit(c, dbus_message_get_path(m), commit);
	r = dbus_message_new_method_return(m);
	b = eaten ? TRUE : FALSE;
	dbus_message_append_args(r, DBUS_TYPE_BOOLEAN, &b, DBUS_TYPE_INVALID);
	dbus_connection_send(c, r, nil);
	dbus_message_unref(r);
	return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult
handlenoop(DBusConnection *c, DBusMessage *m)
{
	DBusMessage *r;

	r = dbus_message_new_method_return(m);
	dbus_connection_send(c, r, nil);
	dbus_message_unref(r);
	return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult
handlereset(DBusConnection *c, DBusMessage *m)
{
	sendreset();
	return handlenoop(c, m);
}

static const char introspectxml[] =
"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n"
" \"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n"
"<node>\n"
" <interface name=\"org.freedesktop.IBus\">\n"
"  <method name=\"CreateInputContext\">\n"
"   <arg direction=\"in\" type=\"s\"/>\n"
"   <arg direction=\"out\" type=\"o\"/>\n"
"  </method>\n"
" </interface>\n"
" <interface name=\"org.freedesktop.IBus.InputContext\">\n"
"  <method name=\"ProcessKeyEvent\">\n"
"   <arg direction=\"in\" type=\"u\"/>\n"
"   <arg direction=\"in\" type=\"u\"/>\n"
"   <arg direction=\"in\" type=\"u\"/>\n"
"   <arg direction=\"out\" type=\"b\"/>\n"
"  </method>\n"
"  <method name=\"FocusIn\"/>\n"
"  <method name=\"FocusOut\"/>\n"
"  <method name=\"Reset\"/>\n"
"  <method name=\"Destroy\"/>\n"
"  <method name=\"SetCapabilities\"><arg direction=\"in\" type=\"u\"/></method>\n"
"  <method name=\"SetCursorLocation\">"
"<arg direction=\"in\" type=\"i\"/><arg direction=\"in\" type=\"i\"/>"
"<arg direction=\"in\" type=\"i\"/><arg direction=\"in\" type=\"i\"/></method>\n"
"  <method name=\"SetEngine\"><arg direction=\"in\" type=\"s\"/></method>\n"
"  <signal name=\"CommitText\"><arg type=\"v\"/></signal>\n"
" </interface>\n"
"</node>\n";

static DBusHandlerResult
handleintrospect(DBusConnection *c, DBusMessage *m)
{
	DBusMessage *r;
	const char *xml = introspectxml;

	r = dbus_message_new_method_return(m);
	dbus_message_append_args(r, DBUS_TYPE_STRING, &xml, DBUS_TYPE_INVALID);
	dbus_connection_send(c, r, nil);
	dbus_message_unref(r);
	return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult
onmsg(DBusConnection *c, DBusMessage *m, void *_)
{
	const char *iface, *member, *path;

	USED(_);
	if(dbus_message_get_type(m) != DBUS_MESSAGE_TYPE_METHOD_CALL)
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	iface = dbus_message_get_interface(m);
	member = dbus_message_get_member(m);
	path = dbus_message_get_path(m);
	if(iface == nil || member == nil || path == nil)
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	if(strcmp(iface, "org.freedesktop.DBus.Introspectable") == 0
	&& strcmp(member, "Introspect") == 0)
		return handleintrospect(c, m);
	if(strcmp(iface, "org.freedesktop.DBus") == 0){
		if(strcmp(member, "Hello") == 0)
			return handlehello(c, m);
		if(strcmp(member, "AddMatch") == 0
		|| strcmp(member, "RemoveMatch") == 0)
			return handlenoop(c, m);
	}
	if(strcmp(iface, "org.freedesktop.IBus") == 0){
		if(strcmp(member, "CreateInputContext") == 0)
			return handlecreate(c, m);
	}
	if(strcmp(iface, "org.freedesktop.IBus.InputContext") == 0){
		if(strcmp(member, "ProcessKeyEvent") == 0)
			return handlekey(c, m);
		if(strcmp(member, "FocusOut") == 0
		|| strcmp(member, "Reset") == 0)
			return handlereset(c, m);
		if(strcmp(member, "FocusIn") == 0
		|| strcmp(member, "Destroy") == 0
		|| strcmp(member, "SetCapabilities") == 0
		|| strcmp(member, "SetCursorLocation") == 0
		|| strcmp(member, "SetEngine") == 0)
			return handlenoop(c, m);
	}
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void
newconn(DBusServer *s, DBusConnection *c, void *_)
{
	DBusObjectPathVTable vt;

	USED(s);
	USED(_);
	if(nconns >= Maxconns)
		return;
	dbus_connection_ref(c);
	dbus_connection_set_watch_functions(c, addwatch, removewatch, togglewatch, nil, nil);
	memset(&vt, 0, sizeof(vt));
	vt.message_function = onmsg;
	dbus_connection_register_fallback(c, "/", &vt, nil);
	conns[nconns++] = c;
}

static void
pruneconns(void)
{
	int i, j;

	j = 0;
	for(i = 0; i < nconns; i++){
		if(dbus_connection_get_is_connected(conns[i])){
			conns[j++] = conns[i];
			continue;
		}
		dbus_connection_unref(conns[i]);
	}
	nconns = j;
}

static int
ibusinit(void)
{
	DBusError err;
	char addr[128];
	char *full;

	if(buildaddrpath(addrfile, sizeof(addrfile)) < 0){
		fprintf(stderr, "strans: ibus: cannot build address path\n");
		return -1;
	}
	unlink(addrfile);
	snprintf(addr, sizeof(addr), "unix:abstract=strans-%d", (int)getpid());
	dbus_error_init(&err);
	srv = dbus_server_listen(addr, &err);
	if(srv == nil){
		fprintf(stderr, "strans: ibus: listen: %s\n", err.message);
		dbus_error_free(&err);
		addrfile[0] = '\0';
		return -1;
	}
	dbus_server_set_new_connection_function(srv, newconn, nil, nil);
	dbus_server_set_watch_functions(srv, addwatch, removewatch, togglewatch, nil, nil);
	full = dbus_server_get_address(srv);
	if(writeaddr(addrfile, full) < 0){
		fprintf(stderr, "strans: ibus: cannot write %s\n", addrfile);
		dbus_free(full);
		dbus_server_disconnect(srv);
		dbus_server_unref(srv);
		srv = nil;
		addrfile[0] = '\0';
		return -1;
	}
	dbus_free(full);
	atexit(unlinkaddr);
	return 0;
}

void
ibusthread(void *_)
{
	struct pollfd pfds[Maxwatches];
	int wi[Maxwatches];
	int i, n, rv;
	unsigned int f;

	USED(_);
	threadsetname("ibus");
	if(ibusinit() < 0)
		return;
	for(;;){
		n = 0;
		for(i = 0; i < nwatches && n < Maxwatches; i++){
			if(watches[i].w == nil)
				continue;
			if(!dbus_watch_get_enabled(watches[i].w))
				continue;
			pfds[n].fd = dbus_watch_get_unix_fd(watches[i].w);
			pfds[n].events = 0;
			f = dbus_watch_get_flags(watches[i].w);
			if(f & DBUS_WATCH_READABLE) pfds[n].events |= POLLIN;
			if(f & DBUS_WATCH_WRITABLE) pfds[n].events |= POLLOUT;
			wi[n] = i;
			n++;
		}
		rv = poll(pfds, n, 200);
		if(rv < 0)
			continue;
		for(i = 0; i < n; i++){
			if(pfds[i].revents == 0)
				continue;
			if(watches[wi[i]].w == nil)
				continue;
			f = 0;
			if(pfds[i].revents & POLLIN) f |= DBUS_WATCH_READABLE;
			if(pfds[i].revents & POLLOUT) f |= DBUS_WATCH_WRITABLE;
			if(pfds[i].revents & POLLHUP) f |= DBUS_WATCH_HANGUP;
			if(pfds[i].revents & POLLERR) f |= DBUS_WATCH_ERROR;
			dbus_watch_handle(watches[wi[i]].w, f);
		}
		for(i = 0; i < nconns; i++)
			while(dbus_connection_dispatch(conns[i]) == DBUS_DISPATCH_DATA_REMAINS)
				;
		pruneconns();
	}
}
