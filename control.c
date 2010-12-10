#include "lisp.h"
#include "save.h"
#include "error.h"

CELLP cond_f(CELLP clauses, CELLP env) 
{
	CELLP key, bodies, result, eval();
	if(clauses->id != _CELL) {
		return (CELLP)error(NEA);
	}
	while(clauses->id == _CELL) {
		if(clauses->car->id != _CELL) {
			return (CELLP)error(CCL);
		}
		int q = on(&env);
		on(&clauses);
		key = eval(clauses->car->car, env); ec;
		off(q);
		if(key != (CELLP)nil) {
			if((bodies = clauses->car->cdr)->id != _CELL) {
				return key;
			}
			while(bodies->id == _CELL) {
			     q = on(&env);
			     on(&clauses);
			     on(&bodies);
			     result = eval(bodies->car, env); ec;
			     off(q);
			     bodies = bodies->cdr;
			}
			return result;
		}
		clauses = clauses->cdr;
	}
	return (CELLP)nil;
}

static CELLP setenv(ATOMP var, CELLP val, CELLP env) {
	while(env->id == _CELL) {
		if(env->car->id != _CELL) {
			return (CELLP)error(EHA);
		}
		if(env->car->car == (CELLP)var) {
			env->car->cdr = val;
			return env;
		}
		env = env->cdr;
	}
	return NULL;
}

CELLP setq_f(CELLP args, CELLP env)
{
	CELLP val, result,setenv(), eval();
	ATOMP var;
	while(args->id == _CELL) {
		if(args->cdr->id != _CELL) {
			return (CELLP)error(NEA);
		}
		var = (ATOMP)args->car;
		int q = on(&args);
		on(&env);
		on((CELLP *)var);
		val = eval(args->cdr->car, env); ec;
		off(q);
		if(var->id != _ATOM) {
			return (CELLP)error(IAA);
		}
		if(var == nil || var == t || var == eofread) {
			return (CELLP)error(CCC);
		}
		q = on(&env);
		on(&args);
		on(&val);
		on((CELLP *)var);
		result = setenv(var, val, env); ec;
		off(q);
		if(result == NULL) {
			var->value = val;
		}
		args = args->cdr->cdr;
	}
	return val;
}


CELLP quote_f(CELLP args, CELLP env) {
	if(args->id != _CELL) {
		return (CELLP)error(NEA);
	}
	return args->car;
}

CELLP de_f(CELLP args, CELLP env) {
	CELLP val, cons();
	ATOMP func;

	if(args->id != _CELL || args->cdr->id != _CELL) {
		return (CELLP)error(NEA);
	}
	if((func = (ATOMP)args->car)->id != _ATOM) {
		return (CELLP)error(IAA);
	}

	int q = on(&args);
	on(&env);
	on((CELLP *)func);
	val = cons((CELLP)lambda, args->cdr); ec;
	off(q);
	func->ftype = _EXPR;
	func->fptr  = val;
	return (CELLP)func;
}

CELLP oblist_f() {
	int i = 0;
	CELLP cp1, cp2, cp3, newcell();
	stackcheck;

	cp1 = *++sp = newcell(); ec;
	for(i = 0; i < TABLESIZ; ++i) {
		if((cp2 = oblist[i]) != (CELLP)nil) break;
	}
	int q = on(&cp1);
	on(&cp2);
	for(; cp2->cdr != (CELLP)nil; cp2 = cp2->cdr) {
		cp1->car = cp2->car;
		cp3 = newcell(); ec;
		cp1->cdr = cp3;
		cp1 = cp3;
	}
	cp1->car = cp2->car;
	for(++i; i < TABLESIZ; ++i) {
		for(cp2 = oblist[i]; cp2 != (CELLP)nil; cp2 = cp2->cdr) {
			cp3 = newcell(); ec;
			cp1->cdr = cp3;
			cp1 = cp3;
			cp1->car = cp2->car;
		}
	}
	off(q);
	return *sp--;
}

CELLP quit_f() {
	exit(0);
}

CELLP reclaim_f()
{
	gbc(ON, ON); ec;
	return (CELLP)nil;
}

CELLP verbos_f(CELLP arg) 
{
	if(arg == (CELLP)nil) {
		if(verbos) return (CELLP)t;
		else 	    return (CELLP)nil;
	}
	if(arg->car == (CELLP)nil) {
		verbos = OFF;
		return (CELLP)nil;
	}
	else {
		verbos = ON;
		return (CELLP)t;
	}
}
