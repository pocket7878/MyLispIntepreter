#include "lisp.h"
static int eval_arg_p(ATOMP func);
static CELLP apply(CELLP func, CELLP args, CELLP env);
static CELLP evallist(CELLP args, CELLP env);
static CELLP push(CELLP keys, CELLP value, CELLP env);
static CELLP atomvalue(ATOMP ap, CELLP env);

CELLP eval(CELLP form, CELLP env)
{
static int e = 0;
static char tabs[100];
	CELLP cp, apply(), atomvalue(), evallist();
	ATOMP func;
	tabs[e] = '\t';
	tabs[++e] = '\0';
	printf("\n%s%d: form=", tabs, e);
	print_s(form, ESCON);
	printf(", env=");
	print_s(env, ESCON);

	switch(form->id) {
		case _ATOM:
			cp = atomvalue((ATOMP)form, env);
			break;
		case _FIX:
		case _FLT:
			printf("\n%s%d: result=(NUM)", tabs, e);
			print_s(form);
			tabs[--e] = '\0';
			return form;
		case _CELL:
			stackcheck;
			//スタックポインタを進める
			++sp;
			func = (ATOMP)form->car;
			if(eval_arg_p(func)) {
				//スタックに引き数を評価した結果を保存する(このバックグラウンドでspは--されている)
				*sp = evallist(form->cdr, env);
				if(err) break;
			}
			else {
				*sp = form->cdr;
			}
			cp = apply((CELLP)func, *sp, env);
			sp--;
			break;
		default:
printf("\n%s%d: result=EORROR", tabs, e);
tabs[--e] = '\0';
			error(ULO);

	}
	if(err == ERR) {
		pri_err(form);
printf("\n%s%d: result=EORROR", tabs, e);
tabs[--e] = '\0';
		return NULL;
	}
printf("\n%s%d: result=", tabs, e);
print_s(cp, ESCON);
tabs[--e] = '\0';
if(e == 0) printf("\n");
	return cp;
}

static int eval_arg_p(ATOMP func) 
{
	if(func->id == _ATOM && func->ftype & _EA) {
		return TRUE;
	}
	if(func->id == _CELL && ((CELLP)func)->car == (CELLP)lambda) {
		return TRUE;
	}
	return FALSE;
}

//function, args, environment!!
static CELLP apply(CELLP func, CELLP args, CELLP env)
{
	CELLP (*funcp)(), bodies, result = (CELLP)nil;
	CELLP bind();
	char funtype;

	//function check
	switch(func->id) {
		case _ATOM:
			//if func is atom => maybe just func
			funtype = ((ATOMP)func)->ftype;
			if(funtype & _UD) {
				return error(UDF);
			}
			if(funtype & _SR) {
				funcp = (CELLP (*)())((ATOMP)func)->fptr;
				if(funtype & _EA) {
					return (*funcp)(args);
				}
				else {
					return (*funcp)(args, env);
				}
			}
			func = ((ATOMP)func)->fptr;
		case _CELL:
			//if func is cell => maybe lambda
			//(lambda (x) <- this must be cell
			if(func->cdr->id != _CELL) {
				return error(IFF);
			}
			//(lambda <- check!!
			if(func->car == (CELLP)lambda) {
				//body (lambda (x) (hoge hoge) <- body!!
				bodies = func->cdr->cdr;
				stackcheck;
				
				//lambda-argsの引き数のそれぞれにargsの値をbindするよ :-)!!
				*++sp = bind(func->cdr->car, args, env); ec;
				for(; bodies->id == _CELL; bodies = bodies->cdr) {
					result = eval(bodies->car, *sp); ec;
				}
				sp--;
				return result;
			}
		default:
			return error(IFF);
	}
}

static CELLP evallist(CELLP args, CELLP env)
{
	CELLP cp1, newcell(), eval();
	//引き数のリストがcellでない場合はおそらくnilなのでnilを返却する
	if(args->id != _CELL) {
		return (CELLP)nil;
	}
	stackcheck;
	//stackに新しいcellを用意する
	*++sp = newcell(); ec;
	//現在のスタックポインタを一旦保存しておく
	cp1 = *sp;
	//保存したcellのcarに引き数の一つ目を評価した物を入れる
	cp1->car = eval(args->car, env); ec;
	//次の引き数に移る
	args = args->cdr;
	//引き数がcell型である限り、処理を進める
	while(args->id == _CELL) {
		//保存したcellのcdrに新しいcellを確保する
		cp1->cdr = newcell(); ec;
		//保存したcellのcdrに評価結果を入れる
		cp1 = cp1->cdr;
		cp1->car = eval(args->car, env); ec;
		args = args->cdr;
	}
	//これを抜けた辞典でスタックにはすべての引き数の評価結果が入っているそしてnilでしめる。
	cp1->cdr = (CELLP)nil;
	//スタックポインタを返し、その後spを一つ減らす
	return *sp--;
}

//keysのそれぞれにvaluesのそれぞれの値をbindするよ！！
CELLP bind(CELLP keys, CELLP values, CELLP env)
{
	CELLP push();
	//keysががnilでなくかつ、keysがatomなら
	if(keys != (CELLP)nil && keys->id == _ATOM) {
		env = push(keys, values, env); ec;
		return env;
	}
	stackcheck;
	*++sp = env;
	while(keys->id == _CELL) {
		if(values->id != _CELL) {
			return error(NEA);
		}
		*sp = push(keys->car, values->car, *sp); ec;
		keys = keys->cdr;
		values = values->cdr;
	}
	if(keys != (CELLP)nil && keys->id == _ATOM) {
		*sp = push(keys, values, *sp); ec;
	}
	return *sp--;
}

static CELLP push(CELLP keys, CELLP value, CELLP env)
{
	CELLP newcell();
	stackcheck;
	*++sp = newcell(); ec;
	(*sp)->cdr = env;
	env = *sp;
	env->car = newcell(); ec;
	env->car->car = keys;
	env->car->cdr = value;
	return *sp--;
}

static CELLP atomvalue(ATOMP ap, CELLP env)
{
	while(env->id == _CELL) {
		if(env->car->id != _CELL) {
			return error(EHA);
		}
		if(env->car->car == (CELLP)ap) {
			return env->car->cdr;
		}
		env = env->cdr;
	}
	return ap->value;
}
