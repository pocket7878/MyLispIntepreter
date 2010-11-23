#include "lisp.h"
#define  forever for(;;) 

CELLP get1arg(CELLP args, NUMP valp) {
	char c;
	CELLP error();

	if(args->id != _CELL)  {
		return error(NEA);
	}
	if((c = args->car->id) == _FIX) {
		valp->id = _FIX;
		valp->value.fix = ((NUMP)(args->car))->value.fix;
		return args->cdr;
	}

	if(c == _FLT) {
		valp->id = _FLT;
		valp->value.flt = ((NUMP)(args->car))->value.flt;
		return args->cdr;
	}
	return error(IAN);
}

static CELLP setfirst(CELLP args,NUMP *npp) {
	CELLP get1arg();
	NUM val, *newnum();
	//引数の一番目がvalに入ってくる、返り値はargs->cdr
	args = get1arg(args, &val); ec;
	*npp = newnum(); ec;
	if(val.id == _FIX) {
		(*npp)->value.fix = val.value.fix;
	} 
	else {
		(*npp)->id = _FLT;
		(*npp)->value.flt = val.value.flt;
	}
	return args;
}

void toflt(NUMP np) {
	if(np->id != _FLT) {
		np->id = _FLT;
		np->value.flt = (double)(np->value.fix);
	}
}

CELLP minus_f(CELLP args) {
	char c;
	NUMP np, newnum();
	CELLP error();

	if(args->id != _CELL) {
		return error(NEA);
	}
	if((c = args->car->id) != _FIX && c != _FLT) {
		return error(IAN);
	}
	np = newnum(); ec;
	if(c == _FIX) {
		np->value.fix = -((NUMP)(args->car))->value.fix;
	}
	else {
		np->id = _FLT;
		np->value.flt = -((NUMP)(args->car))->value.flt;
	}
	return (CELLP)np;
}

CELLP plus_f(CELLP args){
	CELLP setfirst(), get1arg();
	NUM val;
	NUMP np;

	//args->cdrとnpには最初の値がかえってくる
	args = setfirst(args, &np); ec;
	//引数が一つの場合はその引数を返却する
	if(args == (CELLP)nil) {
		return (CELLP)np;
	}
	//引数が複数個ある場合は
	//args->cdr, val = args->cdr->car
	args = get1arg(args, &val); ec;
	//npに足していく	
	if(np->id == _FIX) {
		while(val.id == _FIX) {
			//npに足していく
			np->value.fix += val.value.fix;
			//引数が尽きたら、npをかえす
			if(args == (CELLP)nil) {
				return (CELLP)np;
			}
			//update args
			args = get1arg(args, &val); ec;
		}
		toflt(np);
	}
	//if np->id == _FLT then;
	else {
		toflt(&val);
	}
	forever {
		np->value.flt += val.value.flt;
		if (args == (CELLP)nil) {
			return (CELLP)np;
		}
		args = get1arg(args, &val); ec;
		toflt(&val);
	}
}
