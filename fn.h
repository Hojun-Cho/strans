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
int	mapget(Trie*, Str*, Str*);

Trie*	trieopen(char*);
char*	trieget(Trie*, char*, int, int*);
int	trielookup(Trie*, char*, int, char**, int*);

Lang*	getlang(int);
void	mapinit(char*);
void	dictinit(char*);

void	dictthread(void*);

void	drawthread(void*);
void	imthread(void*);
Emit	transmap(Im*, Rune);
Emit	transko(Im*, Rune);
Emit	transvi(Im*, Rune);
void	backko(Im*);
void	dictsend(Im*, Str*);

void	srvthread(void*);

void*	emalloc(ulong);
void*	erealloc(void*, ulong);

void	fontinit(char*);
void	putfont(u32int*, int, int, int, int, Rune);
