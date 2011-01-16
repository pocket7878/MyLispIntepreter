#include <stdlib.h>//N//
#include "lisp.h"
#include "control.h"//N//
#include "eval.h"//N//
#include "fun.h"//N//
#include "gbc.h"//N//
#include "save.h"
#include "error.h"

CELLP cond_f(CELLP clauses, CELLP env) 
{
	CELLP key, bodies, result, eval();
	if(clauses->id != _CELL) {
		return (CELLP)error(NEA);
	}
	while(clauses->id == _CELL) {
		int q;//N//
		if(clauses->car->id != _CELL) {
			return (CELLP)error(CCL);
		}
		q = on(&env);//N//
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
				on(&key);
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

static CELLP setenvironment(ATOMP var, CELLP val, CELLP env) {
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
	CELLP val, result,setenvironment(), eval();
	ATOMP var;
	while(args->id == _CELL) {
		int q;//N//
		if(args->cdr->id != _CELL) {
			return (CELLP)error(NEA);
		}
		var = (ATOMP)args->car;
		q = on(&args);//N//
		on(&env);
		on((CELLP *)&var);//N//
		val = eval(args->cdr->car, env);//N//
		off(q);//N//
			ec;//N//
		if(var->id != _ATOM) {
			return (CELLP)error(IAA);
		}
		if(var == nil || var == t || var == eofread) {
			return (CELLP)error(CCC);
		}
		q = on(&env);
		on(&args);
		on(&val);
		on((CELLP *)&var);//N//
		result = setenvironment(var, val, env);//N//
		off(q);//N//
			ec;//N//
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
	int q;//N//
	if(args->id != _CELL || args->cdr->id != _CELL) {
		return (CELLP)error(NEA);
	}
	if((func = (ATOMP)args->car)->id != _ATOM) {
		return (CELLP)error(IAA);
	}

	q = on(&args);//N//
	on(&env);
	on((CELLP *)&func);//N//
	val = cons((CELLP)lambda, args->cdr);//N//
	off(q);//N//
		ec;//N//
	func->ftype = _EXPR;
	func->fptr  = val;
	return (CELLP)func;
}

CELLP oblist_f(void) {//N//
	int i;//N//
	CELLP cp1, cp2, cp3, newcell();
	int q;//N//
	stackcheck;

	cp1 = *++sp = newcell(); ec;
	for(i = 0; i < TABLESIZ; ++i) {
		if((cp2 = oblist[i]) != (CELLP)nil) break;
	}
	q = on(&cp1);//N//
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

CELLP quit_f(void) {//N//
	printf("CG times: %d\n",gc_time);
	exit(0);
	return 0;//N//•K—v‚È‚¢‚ªŒx‚ð‚Â‚Ô‚·‚½‚ß
}

CELLP reclaim_f(void)//N//
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
