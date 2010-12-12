#include <io.h>//N//
#include <stdlib.h>//N//
#include <string.h>//N//
#include "lisp.h"
#include "print.h"//N//
#include "error.h"//N//
#include "gbc.h"//N//
#include "read.h"//N//
#include "save.h"
#include <ctype.h>
#define forever for(;;)
static STR getstr(void);//N//
static void getname(STR strp);//N//
static ATOMP ret_atom(void);//N//
static int skipspace(void);//N//
static CELLP escopt(int level) ;
static NUMP mk_num(void);//N//
static hash(STR nam) ;
static ATOMP mk_sub(STR nam) ;
static CELLP mk_list(int level);
static void getcar(CELLP cp, int level);//N//
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
static void getname(STR strp)
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
				return /*(int)error(EOFERR)*/;//N//
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
			return /*(int)error(CTRLIN)*/;//N//
		}
		*strp++ = *txtp++;
	}
	*strp = '\0';
}

static ATOMP ret_atom(void)//N//
{
	uchar nambuf[NAMLEN+ 1];
	ATOMP ap, old_atom(), mk_atom();

	getname(nambuf);
	if((ap = old_atom(nambuf)) == NULL) {
		return mk_atom(nambuf);
	}
	return ap;
}

static int skipspace(void)//N//
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
			//break;//N//
		case '|':
		case '\\':
			return (CELLP)ret_atom(/*ON*/);//N//
			//break;//N//
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
//	STR strcpy();//N//
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
	int q;//N//

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
	q = on(&cp1);//N//
	getcar(cp1, UNDER);
	off(q);//N//
		ec;
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
			q = on(&cp1);//N//
			getcdr(cp1, UNDER);
			off(q);//N//
				ec;
			break;
		}
		q = on(&cp1);//N//
		cp2 = newcell();
		off(q);//N//
			ec;
		q = on(&cp1);//N//
		on(&cp2);//N//
		cp1->cdr = cp2;
		getcar(cp2, UNDER);
		off(q);//N//
			ec;
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

static void getcar(CELLP cp, int level) {//N//
	CELLP cp1, read_s();
	int q = on(&cp);//N//
	cp1 = read_s(level); //ec;//N//
	off(q);//N//
	cp->car = cp1;
}

static int getcdr(CELLP cp, int level) {//N//
	CELLP cp1, read_s(), error();
	int q = on(&cp);//N//
	cp1 = read_s(level); ec;
	off(q);//N//
	cp->cdr = cp1;
	if(skipspace() == NULL) {
		return (int)error(EOFERR);
	}
	if(*txtp != ')' && *txtp != ']') {
		return (int)error(PSEXP);
	}
	return 0;
}

int num(STR x) {
	if(isdigit(*x)) {
		return TRUE;
	}

	if(*x == '-' || *x == '+') {//N//
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
	//return FALSE;//N//
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
		return (CELLP)ret_atom(/*ON*/);//N//
	} else if(iskanji(*txtp)) {
		return (CELLP)ret_atom(/*ON*/);//N//
	} else {
		return (CELLP)error(CTRLIN);
	}
}

//既存のAtomを取り出してくる処理です。
ATOMP old_atom(STR nam) 
{
	int i/* = 0*/;//N//
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

ATOMP mk_atom(STR nam)
{
	ATOMP ap, mk_sub();
	int q;//N//
	ap = mk_sub(nam); ec;
	q = on((CELLP*)&ap);//N//
	intern(ap);
	off(q);//N//
	return ap;
}


void intern(ATOMP ap)//N
{
	int i/* = 0*/;
	CELLP cp, newcell();
	int q = on((CELLP*)&ap);//N//
	cp = newcell(); //ec;//N//
	off(q);//N//
	i = hash(ap->name);
	cp->car = (CELLP)ap;
	cp->cdr = oblist[i];
	oblist[i] = cp;
}
