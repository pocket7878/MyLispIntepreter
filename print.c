#include "lisp.h"
#include <ctype.h>
#define forever for(;;)

static putstr(int mode, STR tp);
print_s(CELLP cp, int mode) {
	if(cp->id != _CELL) {
		pri_atom(cp, mode);
	}
	else {
		fputc('(', cur_fpo);
		forever{
			print_s(cp->car, mode);
			cp = cp->cdr;
			if(cp->id != _CELL) {
				break;
			}
			fputc(' ', cur_fpo);
		}
		if(cp != (CELLP)nil) {
			fprintf(cur_fpo, " . ");
			pri_atom(cp,mode);
		}
		fputc(')', cur_fpo);
	}
}

pri_atom(CELLP cp, int mode) {
	switch(cp->id) {
		case _FIX:
			fprintf(cur_fpo, "%ld", ((NUMP)cp)->value.fix);
			break;
		case _FLT:
			fprintf(cur_fpo, "%#.6g", ((NUMP)cp)->value.flt);
			break;
		case _ATOM:
			putstr(mode, ((ATOMP)cp)->name);
			break;
		default:
			error(ULO);
	}
}

static putstr(int mode, STR tp) {
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
