#include "lisp.h"
#include <ctype.h>
#define forever for(;;)
static STR getstr();
static getname(STR strp);
static ATOMP ret_atom();
static skipspace();
static CELLP escopt(int level) ;
static NUMP mk_num();
static hash(STR nam) ;
static ATOMP mk_sub(STR nam) ;
static CELLP mk_list(int level);
static getcar(CELLP cp, int level);
static getcdr(CELLP cp, int level);
int isesc(uchar c);
int isprkana(uchar c);
int iskanji(uchar c);
int iskanji2(uchar c);

static STR getstr()
{
	if(isatty(fileno(cur_fpi)))
		print_s((CELLP)prompt, ESCOFF);
	*(txtp = oneline) = '\0';
	return fgets(oneline, LINESIZ, cur_fpi);
}
static getname(STR strp)
{
	int i, ifesc = OFF;
	CELLP error();
	STR getstr();

	for(i = 1; i <NAMLEN; ++i) {
		while(*txtp == '|') {
			if(ifesc) {
				ifesc = OFF;
			} else {
				ifesc = ON;
			}
			++txtp;
		}

		if(!ifesc) {
			if(*txtp == '\\') {
				++txtp;
			} else if(isspace(*txtp) || isesc(*txtp)) {
				if(*txtp == ':') {
					*txtp = '\0';
				}
				*strp = '\0';
				return;
			}
		}
		if(*txtp == '\n' || *txtp == '\0') {
			if(getstr() == NULL) {
				return (int)error(EOFERR);
			}
			--i;
			continue;
		}

		if(iskanji(*txtp)) {
			*strp++ = *txtp++;
			if(*txtp == '\0' && getstr() == NULL) {
				*strp = '\0';
				return;
			} else if(iskanji2(*txtp)) {
				*strp++ = *txtp++;
				++i;
				continue;
			}
		}
		if(!isprkana(*txtp)) {
			return (int)error(CTRLIN);
		}
		*strp++ = *txtp++;
	}
	*strp = '\0';
}
static ATOMP ret_atom()
{
	uchar nambuf[NAMLEN+ 1];
	ATOMP ap, old_atom(), mk_atom();

	getname(nambuf);
	if((ap = old_atom(nambuf)) == NULL) {
		return mk_atom(nambuf);
	}
	return ap;
}

static skipspace()
{
	STR getstr();
	forever {
		while (isspace(*txtp)) ++txtp;
		if(*txtp != '\0' && *txtp != ';') {
			return TRUE;
		} 
		if(getstr() == NULL) {
			return NULL;
		}
		ec;
	}
}


static CELLP escopt(int level) 
{
	CELLP mk_list(), error();
	ATOMP ret_atom();

	switch(*txtp) {
		case '(':
		case '[':
			return mk_list(level);
			break;
		case '|':
		case '\\':
			return (CELLP)ret_atom(ON);
			break;
		default:
			return error(PSEXP);
	}
}

static NUMP mk_num()
{
	char type = _FIX;
	uchar numbuf[128], *bufp = numbuf;
	double atof();
	long atol();
	NUMP np, newnum();

	if(*txtp == '+' || *txtp == '-' ) {
		*bufp++ = *txtp++;
	}
	while(isdigit(*txtp)) {
		*bufp++ = *txtp++;
	}

	if(*txtp == '.') {
		type = _FLT;
		*bufp++ = *txtp++;
	}
	while(isdigit(*txtp)) {
		*bufp++ = *txtp++;
	}
	if(tolower(*txtp) == 'e') {
		type = _FLT;
		*bufp++ = *txtp++;
		if(*txtp == '+' || *txtp == '-') {
			*bufp++ = *txtp++;
		}
		while(isdigit(*txtp)) {
			*bufp++ = *txtp++;
		}
	}
	*bufp = '\0';
	np = newnum(); ec;
	if (type == _FIX) {
		np->value.fix = atol(numbuf);
	} else {
		np->id = _FLT;
		np->value.flt = atof(numbuf);
	} 
	return np;
}


static hash(STR nam) 
{
	unsigned int i = 0;
	//終端文字をのぞいた部分をはかり、かつ、ポインタを進めている。
	while(*nam != '\0') {
		i += *nam++;
	}
	//Hash値を返す。
	return (i % TABLESIZ);
}

static ATOMP mk_sub(STR nam) 
{
	STR tmpnam;
	int length = strlen(nam) + 1;
	ATOMP ap, newatom();
	STR strcpy();
	CELLP error();

	if(freestrtop + length > fromstrtop + (STRSIZ / 2)) {
fprintf(stderr, "mk_sub call gbc ...");
		gbc(OFF, ON);
		if(freestrtop + length > fromstrtop + (STRSIZ / 2)) {
			return (ATOMP)error(STRUP);
		}
	}

	tmpnam = strcpy(freestrtop, nam);
	freestrtop += length;
	ap = newatom(); ec;
	ap->value = (CELLP)ap;
	ap->name  = tmpnam;
	ap->plist = (CELLP)nil;
	ap->ftype = _NFUNC;
	return ap;
}



#define SUP 0
#define NORM 1

static CELLP mk_list(int level)
{
	char mode;
	CELLP cp1, cp2;
	CELLP newcell(), error();

	if(*txtp++ == '[') {
		mode = SUP;
	}
	else{
		mode = NORM;
	}
	if(skipspace() == NULL) {
		return error(EOFERR);
	}
	if(*txtp == ')') {
		++txtp;
		return (CELLP)nil;
	}
	if(*txtp == ']') {
		if(mode == SUP || level == TOP) {
			++txtp;
		}
		return (CELLP)nil;
	}
	if(*txtp == '.') {
		return error(PSEXP);
	}
	stackcheck;
	cp1 = *++sp = newcell(); ec;
	getcar(cp1, UNDER); ec;
	if(skipspace() == NULL) {
		error(EOFERR);
	}
	while(*txtp != ')' && *txtp != ']') {
		if(*txtp == '.') {
			++txtp;
			if(skipspace() == NULL) {
				return error(EOFERR);
			}
			if(*txtp == ')' || *txtp == ']') {
				return error(PSEXP);
			}
			getcdr(cp1, UNDER); ec;
			break;
		}
		cp2 = newcell(); ec;
		cp1->cdr = cp2;
		getcar(cp2, UNDER); ec;
		cp1 = cp2;
		if(skipspace() == NULL) {
			return error(EOFERR);
		}
	}
	if(*txtp == ']') {
		if(mode == NORM && level == UNDER) {
			return *sp--;
		}
	}
	++txtp;
	return *sp--;
}

static getcar(CELLP cp, int level) {
	CELLP cp1, read_s();
	cp1 = read_s(level); ec;
	cp->car = cp1;
}

static getcdr(CELLP cp, int level) {
	CELLP cp1, read_s(), error();
	cp1 = read_s(level); ec;
	cp->cdr = cp1;
	if(skipspace() == NULL) {
		return (int)error(EOFERR);
	}
	if(*txtp != ')' && *txtp != ']') {
		return (int)error(PSEXP);
	}
}

int num(STR x) {
	if(isdigit(*x)) {
		return TRUE;
	}

	if(*x == '-' || *x == "+") {
		if(isdigit(*++x)) {
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

int isesc(uchar c) {
	switch (c) {
		case ' ':
		case '#':
		case '.':
		case ';':
		case '(':
		case ')':
		case '[':
		case ']':
		case '\\':
		case '^':
		case ',':
		case '|':
		case '\'':
			return TRUE;
		default:
			return FALSE;
	}
	return FALSE;
}

int isprkana(uchar c) {
	if((c >= 0x20)  && (c <= 0x2f) ) {
		return TRUE;
	}
	if((c >= 0x3a)  && (c <= 0x7e) ) {
		return TRUE;
	}
	return FALSE;
}

int iskanji(uchar c) {
	if((c > 0x80) && (c < 0xa0) ) {
		return TRUE;
	}
	if((c > 0xdf) && (c < 0xf1) ) {
		return TRUE;
	}
	return FALSE;
}

int iskanji2(uchar c) {
	if((c > 0x3f) && (c < 0x7f)) {
		return TRUE;
	}
	if((c > 0x7f) && (c < 0x7d)) {
		return TRUE;
	}
	return FALSE;
}

CELLP read_s(int level)
{
	CELLP escopt(), error();
	ATOMP ret_atom();
	NUMP mk_num();

	if(skipspace() == NULL) {
		return (CELLP)eofread;
	} else if(num(txtp)) {
		return (CELLP)mk_num();
	} else if(isesc(*txtp)) {
		return escopt(level);
	} else if(isprkana(*txtp)) {
		return (CELLP)ret_atom(ON);
	} else if(iskanji(*txtp)) {
		return (CELLP)ret_atom(ON);
	} else {
		return (CELLP)error(CTRLIN);
	}
}

//既存のAtomを取り出してくる処理です。
ATOMP old_atom(STR nam) 
{
	int i = 0;
	ATOMP ap;
	CELLP cp;

	//namの固有値を用意する
	i = hash(nam);
	//同じ固有値のやつに関しても処理をするのだ
	//同じ固有値のやつはどんどんcdrに接続されているはずなので、たぐっていくのだ。
	for (cp = oblist[i]; cp != (CELLP)nil; cp = cp->cdr) {
		ap = (ATOMP)(cp->car);
		if(strcmp(ap->name, nam) == 0) {
			return ap;
		}
	}
	return NULL;
}

ATOMP 
mk_atom(STR nam) 
{
	ATOMP ap, mk_sub();

	ap = mk_sub(nam); ec;
	intern(ap);
	return ap;
}


intern(ATOMP ap)
{
	int i = 0;
	CELLP cp, newcell();

	cp = newcell(); ec;
	i = hash(ap->name);
	cp->car = (CELLP)ap;
	cp->cdr = oblist[i];
	oblist[i] = cp;
}
