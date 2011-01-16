#include "lisp.h"
#include "eval.h"
#include "error.h"
#include "save.h"
#include "gbc.h"
#include "fun.h"
#include "print.h"
#include <stdlib.h>

CELLP progn_f(CELLP args, CELLP env);
CELLP prog_f(CELLP args, CELLP env);
CELLP go_f(CELLP args, CELLP env);
CELLP ret_f(CELLP args);
CELLP catch_f(CELLP args, CELLP env);
CELLP cerr_f(CELLP args, CELLP env);
CELLP throw_f(CELLP args, CELLP env);
CELLP let_f(CELLP args, CELLP env);
CELLP lets_f(CELLP args, CELLP env);
CELLP assoc(CELLP key, CELLP alist);

static int SPECIAL;

CELLP progn_f(CELLP args, CELLP env)
{
	CELLP result, eval();
	int q;
	result = (CELLP)nil;
	q = on(&args);
	on(&env);
	while(args->id == _CELL) {
		result = eval(args->car, env);
		args = args->cdr;
	}
	off(q);
	return result;
}

CELLP prog_f(CELLP args, CELLP env)
{
	CELLP varlist, forms/*, result*/, *currentsp, cp;
	CELLP bind(), eval(), error(), cons(), assoc();
	int q;

	if(args->id != _CELL) {
		return (CELLP)error(NEA);
	}
	if((varlist = args->car)->id != _CELL && varlist != (CELLP)nil) {
		return (CELLP)error(ILV);
	}
	sp += 2;
	stackcheck;
	*sp = env;
	*(sp-1) = (CELLP)nil;	// ‰º‚©‚çˆÚ“®
	q = on(&args);
	on(&env);
	on(&varlist);
//printf("varlist=");
//print_s(varlist, ESCOFF);
	while(varlist->id == _CELL) {
		if(varlist->car->id == _ATOM) {
			*sp = bind(varlist->car, (CELLP)nil, *sp);
			//*sp = cons(cons(varlist->car, (CELLP)nil), *sp);
			ec;
		}
		else if (varlist->car->id == _CELL) {
			*sp = bind(varlist->car->car, (CELLP)nil, *sp);
			//*sp = cons(cons(varlist->car->car, (CELLP)nil), *sp);
			ec;
			if(varlist->car->cdr->id != _CELL) {
				return (CELLP)error(ILV);
			}
			(*sp)->car->cdr = eval(varlist->car->cdr->car, env);
			ec;
		}
		else {
			return error(IAAL);
		}
		varlist = varlist->cdr;
	}
	off(q);
//	*(sp -1) = (CELLP)nil;	// ã‚ÖˆÚ“®
	forms = args->cdr;
	q = on(&forms);
	on(&args);	// ’Ç‰Á
	on(&env);	// ’Ç‰Á
	on(&varlist);	// ’Ç‰Á
	while(forms->id == _CELL) {
		if(forms->car->id == _ATOM) {
			*(sp - 1) = cons(forms, *(sp - 1)); ec;
		}
		forms = forms->cdr;
	}
//	off(q);	// ‰º‚ÉˆÚ“®
	forms = args->cdr;
	currentsp = sp;
//	result = (CELLP)nil;
	while(forms->id == _CELL) {
		if(forms->car->id == _ATOM) {
			forms = forms->cdr;
			continue;
		}
		eval(forms->car, *sp);
		if(SPECIAL == GO) {
			sp = currentsp;
			if((cp = assoc(throwlabel, *(sp - 1))) == NULL || cp->id != _CELL) {
				off(q);	// ’Ç‰Á
				return NULL;
			}
			SPECIAL = 0;
			forms = cp;
		}
		else if(SPECIAL == RET) {
			sp = currentsp-2;
			SPECIAL = 0;
			off(q);	// ’Ç‰Á
			return throwval;
		}
		ec;
		forms = forms->cdr;
	}
	off(q);	// ã‚©‚çˆÚ“®
	sp -= 2;
	return (CELLP)nil;
}

CELLP go_f(CELLP args, CELLP env) {
	CELLP error();
	if(args->id != _CELL) {
		return (CELLP)error(NEA);
	}
	SPECIAL = GO;
	return(throwlabel = args->car);
}

CELLP ret_f(CELLP args) {
	CELLP error();
	SPECIAL = RET;
	if(args->id != _CELL) {
		return (throwval = (CELLP)nil);
	}
	return(throwval = args->car);
}

CELLP catch_f(CELLP args, CELLP env) {
	CELLP bodies, result, *cur_sp, error(), eval();
	int q;

	if(args->id != _CELL) {
		return (CELLP)error(NEA);
	}
	stackcheck;
	q = on(&args);
	on(&env);
	*++sp = eval(args->car, env); 
	off(q);
	ec;
	if((*sp)->id != _ATOM) {
		return (CELLP)error(TTA);
	}
	bodies = args->cdr;
	result = (CELLP)nil;
	throwlabel = throwval = (CELLP)nil;
	cur_sp = sp;
	while(bodies->id == _CELL) {
		q = on(&args);
		on(&env);
		on(&bodies);
		result = eval(bodies->car, env);
		off(q);
		if(err == THROW && throwlabel == *cur_sp) {
			sp = --cur_sp;
			err = NONERR;
			return throwval;
		}
		ec;
		bodies = bodies->cdr;
	}
	sp--;
	return result;
}

CELLP cerr_f(CELLP args, CELLP env) {
	CELLP /*result,*/ *cur_sp, eval();
	NUMP np, newnum();
	int q;

	cur_sp = sp;
	while(args->id == _CELL) {
//		result = eval(args->car, env);
		if(err == ERR || err == ERROK) {
			sp = cur_sp;
			err = NONERR;
			if(err_no == PSEXP) {
				*txtp = '\0';
			}
			q = on(&args);
			on(&env);
			np = newnum(); 
			off(q);
			ec;
			np->value.fix = (long)err_no;
			return (CELLP)np;
		}
			args = args->cdr;
	}
	return (CELLP)nil;
}

CELLP throw_f(CELLP args, CELLP env) {
	CELLP bodies, result, error(), eval();
	int q;

	if(args->id != _CELL) {
		return (CELLP)error(NEA);
	}
	stackcheck;
	q = on(&args);
	on(&env);
	*++sp = eval(args->car, env); 
	off(q);
	ec;
	if((*sp)->id != _ATOM) {
		return (CELLP)error(TTA);
	}
	throwlabel = *sp--;
	bodies = args->cdr;
	result = (CELLP)nil;
	while(bodies->id == _CELL) {
		q = on(&args);
		on(&env);
		on(&bodies);
		result = eval(bodies->car, env); 
		off(q);
		ec;
		bodies = bodies->cdr;
	}
	throwval = result;
	err = THROW;
	return NULL;
}

CELLP let_f(CELLP args, CELLP env) {
	CELLP list, bodies, error(), bind(), eval();
	int q;
	if(args->id != _CELL) {
		return (CELLP)error(NEA);
	}
	if((list = args->car)->id != _CELL && list != (CELLP)nil) {
		return (CELLP)error(ILV);
	}
	stackcheck;
	*++sp = env;
	while(list->id == _CELL) {
		if(list->car->id == _ATOM) {
			q = on(&args);
			on(&env);
			on(&list);
			*sp = bind(list->car, (CELLP)nil, *sp); 
			off(q);
			ec;
		}
		else if(list->car->id == _CELL) {
			if(list->car->cdr->id != _CELL) {
				return (CELLP)error(ILV);
			}
			q = on(&args);
			on(&env);
			on(&list);
			*sp = bind(list->car->car, (CELLP)nil, *sp); 
			off(q);
			ec;
			q = on(&args);
			on(&env);
			on(&list);
			(*sp)->car->cdr = eval(list->car->cdr->car, env);
			off(q);
			 ec;
		}
		else {
			return (CELLP)error(IAAL);
		}
		list = list->cdr;
	}
	bodies = args->cdr;
	while(bodies->id == _CELL) {
		q = on(&args);
		on(&env);
		on(&list);
		on(&bodies);
		list = eval(bodies->car, *sp); ec;
		off(q);
		bodies = bodies->cdr;
	}
	sp--;
	return list;
}

CELLP lets_f(CELLP args, CELLP env) {
	CELLP list, bodies, error(), bind(), eval();
	int q;
	if(args->id != _CELL) {
		return (CELLP)error(NEA);
	}
	if((list = args->car)->id != _CELL && list != (CELLP)nil) {
		return (CELLP)error(ILV);
	}
	sp += 2;
	stackcheck;
	*sp = env;
	*(sp - 1) = (CELLP)nil;
	while(list->id == _CELL) {
		if(list->car->id == _ATOM) {
			q = on(&args);
			on(&env);
			on(&list);
			*sp = bind(list->car, (CELLP)nil, *sp);
			off(q);
			ec;
		}
		else if (list->car->id == _CELL) {
			if(list->car->cdr->id != _CELL) {
				return (CELLP)error(ILV);
			}
			q = on(&args);
			on(&env);
			on(&list);
			*(sp - 1) = eval(list->car->cdr->car, *sp); 
			off(q);
			ec;
			q = on(&args);
			on(&env);
			on(&list);
			*sp = bind(list->car->car, *(sp - 1), *sp); 
			off(q);
			ec;
		}
		else {
			return (CELLP)error(IAAL);
		}
		list = list->cdr;
	}
	bodies = args->cdr;
	while(bodies->id == _CELL) {
		q = on(&args);
		on(&env);
		on(&list);
		on(&bodies);
		list = eval(bodies->car, *sp); 
		off(q);
		ec;
		bodies = bodies->cdr;
	}
	sp -= 2;
	return list;
}

CELLP assoc(CELLP key, CELLP alist) {
	while(alist->id == _CELL) {
		if(alist->car->id != _CELL) {
			return (CELLP)error(IASSL);
		}
		if(alist->car->car == key) {
			return alist->car;
		}
		alist = alist->cdr;
	}
	return NULL;
}
