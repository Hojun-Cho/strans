// req: 4 bytes [type, mod, key lo, key hi]
// resp: 2+n bytes [type, n, ...]

enum
{
	Kspec = 0xf000,
	Kback = Kspec|0x08,
	Ktab = Kspec|0x09,
	Kret = Kspec|0x0d,
	Kesc = Kspec|0x1b,
	Kup = Kspec|0x52,
	Kdown = Kspec|0x54,
	Kshift = Kspec|0xe1,
	Ksuper = Kspec|0xeb,

	Mshift = 1<<0,
	Mctrl = 1<<2,
	Malt = 1<<3,
	Msuper = 1<<6,
};
