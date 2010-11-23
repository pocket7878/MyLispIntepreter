#ifndef _LIST_H_
#define _LIST_H_

#include <stdio.h>

typedef unsigned char uchar;
typedef uchar *STR;

typedef struct cell {
	char id;
	char cpflag;
	//コピー先へのポインター
	struct cell *forwarding;
	struct cell *car;
	struct cell *cdr;
} CELL;

typedef CELL *CELLP;

typedef struct atom {
	char id;
	char cpflag;
	CELLP forwarding;
	CELLP value;
	CELLP plist;
	STR name;
	char ftype;
	CELLP fptr;
} ATOM;

typedef ATOM *ATOMP;

typedef struct num {
	char id;
	char cpflag;
	CELLP forwarding;
	union body {
		struct num *ptr;
		long fix;
		double flt;
	} value;
} NUM;

typedef NUM *NUMP;

#define _CELL	1
#define _ATOM	2
#define _FIX	3
#define _FLT	4

#define _NFUNC	0x01
#define _SUBR	0x06
#define _EXPR	0x04
#define _FSUBR	0x02

#define _UD	0x01
#define _SR	0x02
#define _EA	0x04

#define CELLSIZ	5000
#define	ATOMSIZ	500
#define STRSIZ	3000
#define	NUMSIZ	30
#define	TABLESIZ	64
#define LINESIZ	100
#define NAMLEN	100


#define ESCON	1
#define	ESCOFF	0
#define ON	1
#define OFF	0

#define TOP 0
#define UNDER 1

#define TRUE (-1)
#define FALSE 0

#define NONERR 0
#define ERR	1
#define ERROK	(-1)

#define ec if(err)return(NULL)

#define STRUP	1
#define NUMUP	2
#define ATOMUP	3
#define CELLUP	4
#define ULO	5
#define PSEXP	6
#define CTRLIN	7
#define UDF	8
#define IFF	9
#define NEA 10
#define	IAA	11
#define IAN	12
#define IAL	13
#define IAAL	14
#define IALN	15
#define IAF	17
#define	IAFL	18
#define ILS	19
#define IASSL	20
#define IPL	21
#define EIA	22
#define EHA	23
#define CCL	24
#define EOFERR	25
#define CCC	26
#define UNDEF	27

#define NONMRK (_UD | _SR)
#define USED 0x80
#define NOTCOPIED 0x00
#define COPIED 0x01
#define FREE (~USED)
#define STACKSIZ 0x0500
#define stackcheck if(sp>=stacktop+STACKSIZ){error(STACKUP);return(NULL);}
#define STACKUP 28

#ifdef MAIN
#include "defvar.h"
#else
#include "var.h"
#endif

#endif
