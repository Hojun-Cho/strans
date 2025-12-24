#include <u.h>
#include <libc.h>
#include <thread.h>
#include <bio.h>
#include "ipc.h"

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

enum
{
	LangEN	= 0x14,
	LangJP	= 0x0e,
	LangJPK	= 0x0b,
	LangKO	= 0x13,

	Fontsz	= 32,
	Fontbase	= 4,
	Nglyphs	= 65536,
};

enum
{
	Maxclients	= 64,
	Maxkouho	= 32,
	Maxdisp		= 8,
	Imgw		= 512,
	Imgh		= Maxdisp * Fontsz + 8,

	Colfg	= 0x000000,
	Colbg	= 0xffffff,
	Colsel	= 0xcccccc,
};

typedef struct Str Str;
struct Str
{
	Rune	r[64];
	int	n;
};

typedef struct Emit Emit;
struct Emit
{
	int	eat;
	int	flush;
	Str	s;
	Str	next;
	Str	dict;
};

typedef struct Hnode Hnode;
struct Hnode
{
	int	filled;
	int	next;
	char	*key;
	int	klen;
	char	*kana;
	int	kanalen;
};

typedef struct Hmap Hmap;
struct Hmap
{
	int	nbs;
	int	nsz;
	int	len;
	int	cap;
	uchar	*nodes;
};

typedef struct Lang Lang;
typedef struct Im Im;
struct Lang
{
	int	lang;
	char	*mapname;
	char	*dictname;
	Hmap	*map;
	Hmap	*dict;
	Emit	(*trans)(Im*, Rune);
};

struct Im
{
	Lang	*l;
	Str	line;
	Str	kouho[Maxkouho];
	int	nkouho;
	int	sel;
};

typedef struct Drawcmd Drawcmd;
struct Drawcmd
{
	Str	preedit;
	Str	kouho[Maxkouho];
	int	nkouho;
	int	sel;
};

typedef struct Keyreq Keyreq;
struct Keyreq
{
	int	fd;
	u32int	ks;
	u32int	mod;
};

typedef struct Dictreq Dictreq;
struct Dictreq
{
	Str	key;
	Str	line;
	int	lang;
};

typedef struct Dictres Dictres;
struct Dictres
{
	Str	key;
	Str	kouho[Maxkouho];
	int	nkouho;
};

extern Lang langs[];
extern int nlang;
extern Channel *drawc;
extern Channel *keyc;
extern Channel *dictreqc;
extern Channel *dictresc;
