#include "lisp.h"
#include "error.h"//N//
#include "print.h"
#include "read.h"//N//
#include "save.h"
#include <ctype.h>
#define forever for(;;)

static void putstr(int mode, STR tp);//N//

void print_s(CELLP cp, int mode) {
//printf("[print_s: id=%x]", (int)cp->id);
	if(cp->id != _CELL) {
		pri_atom(cp, mode);
	}
	else {
//fprintf(cur_fpo, "[%p]", cp);
		fputc('(', cur_fpo);
		forever{
			print_s(cp->car, mode);
			cp = cp->cdr;
			if(cp->id != _CELL) {
				break;
			}
			fputc(' ', cur_fpo);
//fprintf(cur_fpo, "cdr[%p]", cp);
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
