#include "lisp.h"
#include "error.h"//N//
#include "print.h"
#include "read.h"//N//
#include "save.h"
#include <ctype.h>
#define forever for(;;)


#define ISCELL1P(x) (C1 <= (x) && (x) < C1 + (CELLSIZ / 2))
#define ISATOM1P(x) (A1 <= (x) && (x) < A1 + (ATOMSIZ / 2))
#define ISNUM1P(x)  (N1  <= (x) && (x) < N1  + (NUMSIZ / 2))
#define ISCELL2P(x) (C2 <= (x) && (x) < C2 + (CELLSIZ / 2))
#define ISATOM2P(x) (A2 <= (x) && (x) < A2 + (ATOMSIZ / 2))
#define ISNUM2P(x)  (N2  <= (x) && (x) < N2  + (NUMSIZ / 2))
#define ISOLDCELLP(x) (old_freecelltop <= (x) && (x) < old_freecelltop + CELLSIZ)
#define ISOLDATOMP(x) (old_freeatomtop <= (x) && (x) < old_freeatomtop + ATOMSIZ)
#define ISOLDNUMP(x) (old_freenumtop <= (x) && (x) < old_freenumtop + NUMSIZ)

#define ISSYSATOMP(x) (freesysatom <= (x) && (x) < freesysatom + SYSATOMS)

static void putstr(int mode, STR tp);//N//

void pp(void* cp)
{
	char* c = "E";
	if(ISOLDATOMP((ATOMP)cp))
	{
		c = "A";
	}
	else if(ISOLDCELLP(cp))
	{
		c = "C";
	}
	else if(ISOLDNUMP((NUMP)cp))
	{
		c = "N";
	}
	else if(ISCELL1P(cp))
	{
		c = "c1";
	}
	else if(ISATOM1P((ATOMP)cp))
	{
		c = "a1";
	}
	else if(ISNUM1P((NUMP)cp))
	{
		c = "n1";
	}
	else if(ISCELL2P(cp))
	{
		c = "c2";
	}
	else if(ISATOM2P((ATOMP)cp))
	{
		c = "a2";
	}
	else if(ISNUM2P((NUMP)cp))
	{
		c = "n2";
	}
	else if(ISSYSATOMP(cp))
	{
		c = "S";
	}
	else
	{
		c = "O";
	}
	printf("[%p:%s]", cp, c);
}

void print_s(CELLP cp, int mode) {
//printf("[print_s: id=%x]", (int)cp->id);
	if(cp->id != _CELL) {
		pri_atom(cp, mode);
	}
	else {
//fprintf(cur_fpo, "[%p]", cp);
//pp(cp);
		fputc('(', cur_fpo);
		forever{
			print_s(cp->car, mode);
			cp = cp->cdr;
			if(cp->id != _CELL) {
				break;
			}
			fputc(' ', cur_fpo);
//fprintf(cur_fpo, "cdr[%p]", cp);
//printf("-cdr-");
//pp(cp);
		}
		if(cp != (CELLP)nil) {
			fprintf(cur_fpo, " . ");
			pri_atom(cp,mode);
		}
		fputc(')', cur_fpo);
	}
}

void pri_atom(CELLP cp, int mode) {
//fprintf(cur_fpo, "[%p]", cp);
//pp(cp);
	switch(cp->id) {
		case _FIX:
			fprintf(cur_fpo, "%ld", ((NUMP)cp)->value.fix);
			break;
		case _FLT:
			fprintf(cur_fpo, "%#.6g", ((NUMP)cp)->value.flt);
			break;
		case _ATOM:
			putstr(mode, ((ATOMP)cp)->name);
//printf("fptr(%p):", ((ATOMP)cp)->fptr);
//if(((ATOMP)cp)->ftype == _EXPR) print_s(((ATOMP)cp)->fptr->car, ESCOFF);
			break;
		default:
			error(ULO);
	}
}

static void putstr(int mode, STR tp) {//N//
	if(mode == ESCOFF) {
		fprintf(cur_fpo, "%s", tp);
	} else if(*tp == '\0') {
		fprintf(cur_fpo, "||");
	} else {
		if(num(tp)) {
			fputc('\\', cur_fpo);
		} 
		do {
			if(iskanji(*tp) && iskanji2(*(tp+1))) {
				fputc(*tp++, cur_fpo);
				fputc(*tp++, cur_fpo);
			} else if(!isprkana(*tp)) {
				fprintf(cur_fpo, "#\\%03d", *tp++);
			} else {
				if(isesc(*tp)) {
					fputc('\\', cur_fpo);
				}
				fputc(*tp++, cur_fpo);
			}
		} while(*tp != '\0');
	}
}
