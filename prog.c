#include "lisp.h"
#include "eval.h"
#include "error.h"
#include "save.h"
#include "gbc.h"
#include "fun.h"
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
	CELLP varlist, forms, result, *currentsp, cp;
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
	q = on(&args);
	on(&env);
	on(&varlist);
	while(varlist->id == _CELL) {
		if(varlist->car->id == _ATOM) {
			*sp = bind(varlist->car, (CELLP)nil, *sp);
			ec;
		}
		else if (varlist->car->id == _CELL) {
			*sp = bind(varlist->car->car, (CELLP)nil, *sp);
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
	*(sp -1) = (CELLP)nil;
	forms = args->cdr;
	q = on(&forms);
	while(forms->id == _CELL) {
		if(forms->car->id == _ATOM) {
			*(sp - 1) = cons(forms, *(sp - 1)); ec;
		}
		forms = forms->cdr;
	}
	off(q);
	forms = args->cdr;
	currentsp = sp;
	result = (CELLP)nil;
	while(forms->id == _CELL) {
		if(forms->car->id == _ATOM) {
			forms = forms->cdr;
			continue;
		}
		eval(forms->car, *sp);
		if(err == GO) {
			sp = currentsp;
			if(!(cp = assoc(throwlabel, *(sp - 1))) || cp->id != _CELL) {
				return NULL;
			}
			err = 0;
			forms = cp;
		}
		else if(err == RET) {
			sp = currentsp-2;
			err = 0;
			return throwval;
		}
		ec;
		forms = forms->cdr;
	}
	sp -= 2;
	return (CELLP)nil;
}

CELLP go_f(CELLP args, CELLP env) {
	CELLP error();
	if(args->id != _CELL) {
		return (CELLP)error(NEA);
	}
	err = GO;
	return(throwlabel = args->car);
}

CELLP ret_f(CELLP args) {
	CELLP error();
	err = RET;
	if(args->id != _CELL) {
		return (throwval = (CELLP)nil);
	}
	return(throwval = args->car);
}

CELLP catch_f(CELLP args, CELLP env) {
	CELLP bodies, result, *cur_sp, error(), eval();
	if(args->id != _CELL) {
		return (CELLP)error(NEA);
	}
	stackcheck;
	*++sp = eval(args->car, env); ec;
	if((*sp)->id != _ATOM) {
		return (CELLP)error(TTA);
	}
	bodies = args->cdr;
	result = (CELLP)nil;
	throwlabel = throwval = (CELLP)nil;
	cur_sp = sp;
	while(bodies->id == _CELL) {
		result = eval(bodies->car, env);
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
	CELLP result, *cur_sp, eval();
	NUMP np, newnum();

	cur_sp = sp;
	while(args->id == _CELL) {
		result = eval(args->car, env);
		if(err == ERR || err == ERROK) {
			sp = cur_sp;
			err = NONERR;
			if(err_no == PSEXP) {
				*txtp = '\0';
			}
				np = newnum(); ec;
				np->value.fix = (long)err_no;
				return (CELLP)np;
		}
			args = args->cdr;
	}
	return (CELLP)nil;
}

CELLP throw_f(CELLP args, CELLP env) {
	CELLP bodies, result, error(), eval();

	if(args->id != _CELL) {
		return (CELLP)error(NEA);
	}
	stackcheck;
	*++sp = eval(args->car, env); ec;
	if((*sp)->id != _ATOM) {
		return (CELLP)error(TTA);
	}
	throwlabel = *sp--;
	bodies = args->cdr;
	result = (CELLP)nil;
	while(bodies->id == _CELL) {
		result = eval(bodies->car, env); ec;
		bodies = bodies->cdr;
	}
	throwval = result;
	err = THROW;
	return NULL;
}

CELLP let_f(CELLP args, CELLP env) {
	CELLP list, bodies, error(), bind(), eval();
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
			*sp = bind(list->car, (CELLP)nil, *sp); ec;
		}
		else if(list->car->id == _CELL) {
			if(list->car->cdr->id != _CELL) {
				return (CELLP)error(ILV);
			}
			*sp = bind(list->car->car, (CELLP)nil, *sp); ec;
			(*sp)->car->cdr = eval(list->car->cdr->car, env); ec;
		}
		else {
			return (CELLP)error(IAAL);
		}
		list = list->cdr;
	}
	bodies = args->cdr;
	while(bodies->id == _CELL) {
		list = eval(bodies->car, *sp); ec;
		bodies = bodies->cdr;
	}
	sp--;
	return list;
}

CELLP lets_f(CELLP args, CELLP env) {
	CELLP list, bodies, error(), bind(), eval();
	if(args->id != _CELL) {
		return (CELLP)error(NEA);
	}
	if((list = args->car)->id != _CELL && list != (CELLP)nil) {
		return (CELLP)error(ILV);
	}
	sp += 2;
	stackcheck;
	*sp = env;
	while(list->id == _CELL) {
		if(list->car->id == _ATOM) {
			*sp = bind(list->car, (CELLP)nil, *sp); ec;
		}
		else if (list->car->id == _CELL) {
			if(list->car->cdr->id != _CELL) {
				return (CELLP)error(ILV);
			}
			*(sp - 1) = eval(list->car->cdr->car, *sp); ec;
			*sp = bind(list->car->car, *(sp - 1), *sp); ec;
		}
		else {
			return (CELLP)error(IAAL);
		}
		list = list->cdr;
	}
	bodies = args->cdr;
	while(bodies->id == _CELL) {
		list = eval(bodies->car, *sp); ec;
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
