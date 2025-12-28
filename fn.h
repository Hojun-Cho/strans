void	die(char*, ...);

void	sinit(Str*, char*, int);
void	sclear(Str*);
void	sputr(Str*, Rune);
void	spopr(Str*);
void	sappend(Str*, Str*);
int	scmp(Str*, Str*);
int	stoutf(Str*, char*, int);
Rune	slastr(Str*);

Hmap*	hmapalloc(int, int);
int	hmapset(Hmap**, Str*, Str*);
Hnode*	hmapget(Hmap*, Str*);
int	mapget(Hmap*, Str*, Str*);

Lang*	getlang(int);
void	mapinit(char*);
void	dictinit(char*);

void	dictthread(void*);

void	drawthread(void*);
void	imthread(void*);
Emit	trans(Im*, Rune);

void	srvthread(void*);

void*	emalloc(ulong);
void*	erealloc(void*, ulong);

void	fontinit(char*);
void	putfont(u32int*, int, int, int, int, Rune);
