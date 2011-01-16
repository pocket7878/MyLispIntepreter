#include "lisp.h"
#include "eval.h"
#include "fun.h"
#include "save.h"

#include "print.h"

CELLP eval_f(CELLP args, CELLP env);
CELLP apply_f(CELLP args, CELLP env);
CELLP rplacd_f(CELLP args);
CELLP lessp_f(CELLP args);
CELLP and_f(CELLP args, CELLP env);

static CELLP append_f(CELLP left, CELLP right);

// (eval form env)
CELLP eval_f(CELLP args, CELLP env)
{
	CELLP local_env, result;
	int q = on(&args);
	on(&env);
//printf("eval_f: args = ");print_s(args, ESCOFF);printf(", env = ");print_s(env, ESCOFF);printf("\n");
	local_env = append_f(args->cdr->car, env);
	on(&local_env);
	result = eval(eval(args->car, local_env), local_env);
	off(q);
	return result;
}

// (apply func arg1 ... argn)
CELLP apply_f(CELLP args, CELLP env)
{
	CELLP local_env, result, func, params;
	int q = on(&args);
	on(&env);
	func = eval(args->car, env);
	on(&func);
	result = eval(cons(func, args->cdr), env);
	off(q);
	return result;
}

// (rplacd left right)
CELLP rplacd_f(CELLP args)
{
	args->car->cdr = args->cdr->car;
	return args->car;
}

// (lessp n1 n2)
CELLP lessp_f(CELLP args)
{
	NUMP n1 = (NUMP)(args->car), n2 = (NUMP)(args->cdr->car);
	if(n1->id == _FIX && n2->id == _FIX)
	{
		return n1->value.fix < n2->value.fix ? (CELLP)t : (CELLP)nil;
	}
	else if(n1->id == _FIX && n2->id == _FLT)
	{
		return n1->value.fix < n2->value.flt ? (CELLP)t : (CELLP)nil;
	}
	else if(n1->id == _FLT && n2->id == _FIX)
	{
		return n1->value.flt < n2->value.fix ? (CELLP)t : (CELLP)nil;
	}
	else if(n1->id == _FLT && n2->id == _FIX)
	{
		return n1->value.flt < n2->value.flt ? (CELLP)t : (CELLP)nil;
	}
	else
	{
		return (CELLP)nil;
	}
}

// (and p1 p2 ... pn)
CELLP and_f(CELLP args, CELLP env)
{
	CELLP cp, result = (CELLP)t;
	for(cp = args; cp != (CELLP)nil && result == (CELLP)t; cp = cp->cdr)
	{
		int q = on(&cp); on(&args); on(&env);
		if(eval(cp->car, env) == (CELLP)nil)
		{
			result = (CELLP)nil;
		}
		off(q);
	}
	return result;
}

// (or p1 p2 ... pn)
CELLP or_f(CELLP args, CELLP env)
{
	CELLP cp, result = (CELLP)nil;
	for(cp = args; cp != (CELLP)nil && result == (CELLP)nil; cp = cp->cdr)
	{
		int q = on(&cp); on(&args); on(&env);
		if(eval(cp->car, env) != (CELLP)nil)
		{
			result = (CELLP)t;
		}
		off(q);
	}
	return result;
}

static CELLP append_f(CELLP left, CELLP right)
{
	if(left == (CELLP)nil)
	{
		return right;
	}
	else
	{
		CELLP result;
		int q = on(&left);
		on(&right);
		result = cons(left->car, append_f(left->cdr, right));
		off(q);
		return result;
	}
}
