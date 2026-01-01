#!/usr/bin/env python3

# tone marks: s=rising f=falling r=hook x=tilde j=dot
tone = {
	's': str.maketrans("aăâeêioôơuưy", "áắấéếíóốớúứý"),
	'f': str.maketrans("aăâeêioôơuưy", "àằầèềìòồờùừỳ"),
	'r': str.maketrans("aăâeêioôơuưy", "ảẳẩẻểỉỏổởủửỷ"),
	'x': str.maketrans("aăâeêioôơuưy", "ãẵẫẽễĩõỗỡũữỹ"),
	'j': str.maketrans("aăâeêioôơuưy", "ạặậẹệịọộợụựỵ"),
}

# modified vowels: input -> output
modvowel = [
	("aw", "ă"),
	("aa", "â"),
	("ee", "ê"),
	("oo", "ô"),
	("ow", "ơ"),
	("uw", "ư"),
]

# modified cons: input -> output
modcons = [
	("dd", "đ"),
]

upper = str.maketrans(
	"aăâeêioôơuưyđáắấéếíóốớúứýàằầèềìòồờùừỳảẳẩẻểỉỏổởủửỷãẵẫẽễĩõỗỡũữỹạặậẹệịọộợụựỵ",
	"AĂÂEÊIOÔƠUƯYĐÁẮẤÉẾÍÓỐỚÚỨÝÀẰẦÈỀÌÒỒỜÙỪỲẢẲẨẺỂỈỎỔỞỦỬỶÃẴẪẼỄĨÕỖỠŨỮỸẠẶẬẸỆỊỌỘỢỤỰỴ")

def addtone(v, t):
	return v.translate(tone[t])

def emit(input, output):
	print(f"{input}\t{output}")
	def up(s):
		c = s[0].translate(upper)
		if c == s[0]:
			c = s[0].upper()
		return c + s[1:]
	print(f"{up(input)}\t{up(output)}")

def vowel1():
	for v in "aeiouy":
		emit(v, v)
		for t in tone:
			emit(v+t, addtone(v, t))

def vowel2():
	# input, output, vowel
	tab = [
		("oa", "oa", "a"), ("oe", "oe", "e"), ("ai", "ai", "a"),
		("ao", "ao", "a"), ("au", "au", "a"), ("ay", "ay", "a"),
		("eu", "eu", "e"), ("iu", "iu", "i"), ("oi", "oi", "o"),
		("ui", "ui", "u"), ("uy", "uy", "y"),
		("iee", "iê", "ê"), ("yee", "yê", "ê"), ("uoo", "uô", "ô"),
		("uow", "ươ", "ơ"), ("uaa", "uâ", "â"), ("oaw", "oă", "ă"),
		("uwa", "ưa", "a"), ("uwow", "ươ", "ơ"),
	]
	for i, o, v in tab:
		emit(i, o)
		for t in tone:
			emit(i+t, o.replace(v, addtone(v, t), 1))
	emit("ie", "ie")
	emit("ye", "ye")
	emit("uo", "uo")
	emit("ua", "ua")

def vowel3():
	# input, output, vowel
	tab = [
		("ieeu", "iêu", "ê"), ("yeeu", "yêu", "ê"),
		("uooi", "uôi", "ô"), ("uowi", "ươi", "ơ"),
		("oai", "oai", "a"), ("oay", "oay", "a"),
		("uyee", "uyê", "ê"),
	]
	for i, o, v in tab:
		emit(i, o)
		for t in tone:
			emit(i+t, o.replace(v, addtone(v, t), 1))
	emit("uya", "uya")

def modvowels():
	for i, o in modvowel:
		emit(i, o)

def modconss():
	for i, o in modcons:
		emit(i, o)

def mod1tone():
	# aw+s -> ắ
	for i, o in modvowel:
		for t in tone:
			emit(i+t, addtone(o, t))

modvowel = [
	("aw", "ă"),
	("aa", "â"),
	("ee", "ê"),
	("oo", "ô"),
	("ow", "ơ"),
	("uw", "ư"),
]
def tone1mod():
	# a+s+w -> ắ
	for i, o in modvowel:
		for t in tone:
			emit(i[0]+t+i[1], addtone(o, t))

def tone2mod():
	# ie+s+e -> iế
	# input, output, vowel, suffix
	tab = [
		("ie", "iê", "ê", "e"), ("ye", "yê", "ê", "e"),
		("uo", "uô", "ô", "o"), ("uo", "ươ", "ơ", "w"),
		("ua", "uâ", "â", "a"), ("oa", "oă", "ă", "w"),
		("uwo", "ươ", "ơ", "w"),
	]
	for i, o, v, s in tab:
		for t in tone:
			emit(i+t+s, o.replace(v, addtone(v, t), 1))

def escape():
	# aww -> aw
	for e in ["aw", "aa", "ee", "oo", "ow", "uw", "dd"]:
		emit(e+e[-1], e)

def final():
	# codas
	coda = ["c", "m", "n", "p", "t", "ch", "ng", "nh"]
	# input, output, vowel
	tab = [
		("a", "a", "a"), ("e", "e", "e"), ("i", "i", "i"),
		("o", "o", "o"), ("u", "u", "u"), ("y", "y", "y"),
		("aw", "ă", "ă"), ("aa", "â", "â"), ("ee", "ê", "ê"),
		("oo", "ô", "ô"), ("ow", "ơ", "ơ"), ("uw", "ư", "ư"),
		("iee", "iê", "ê"), ("yee", "yê", "ê"), ("uoo", "uô", "ô"),
		("uow", "ươ", "ơ"), ("uaa", "uâ", "â"), ("oaw", "oă", "ă"),
		("ai", "ai", "a"), ("ao", "ao", "a"), ("au", "au", "a"),
		("ay", "ay", "a"), ("oa", "oa", "a"), ("oi", "oi", "o"),
		("ui", "ui", "u"), ("uy", "uy", "y"),
	]
	for i, o, v in tab:
		for c in coda:
			emit(i+c, o+c)
			for t in tone:
				emit(i+c+t, o.replace(v, addtone(v, t), 1)+c)

vowel1()
vowel2()
vowel3()
modvowels()
modconss()
mod1tone()
tone1mod()
tone2mod()
escape()
final()
