#include <string.h>//N//
#include "lisp.h"
#include "fun.h"//N//
#include "gbc.h"//N//
#include "read.h"//N//
#include "error.h"//N//
#include "save.h"
#define forever for(;;)

CELLP car_f(CELLP args)
{
	if(args->id != _CELL) {
	     return (CELLP)error(NEA);
	}
	if(args->car == (CELLP)nil) {
		return (CELLP)nil;
	}
	if(args->car->id != _CELL) {
	     return (CELLP)error(IAL);
	}
	return args->car->car;
}

CELLP cdr_f(CELLP args) 
{
	if(args->id != _CELL) {
	     return (CELLP)error(NEA);
	}
	if(args->car == (CELLP)nil) {
		return (CELLP)nil;
	}
	if(args->car->id != _CELL) {
	     return (CELLP)error(IAL);
	}
	return args->car->cdr;
}

CELLP cons_f(CELLP args)
{
	CELLP cp, cons();
	int q;//N//
	if(args->id != _CELL || args->cdr->id != _CELL) {
	     return (CELLP)error(NEA);
	}
	q = on(&args);//N//
	cp = cons(args->car, args->cdr->car); ec;
	off(q);
	return cp;
}

CELLP cons(CELLP arg1, CELLP arg2) 
{
	CELLP cp;
	int q = on(&arg1);
	on(&arg2);
	cp = newcell(); ec;
	off(q);
	cp->car = arg1;
	cp->cdr = arg2;
	return cp;
}

CELLP eq_f(CELLP args)
{
	if(args->id != _CELL || args->cdr->id != _CELL) {
	     return (CELLP)error(NEA);
	}
	if(args->car == args->cdr->car) {
		return (CELLP)t;
	}
	return (CELLP)nil;
}

CELLP atom_f(CELLP args)
{
	if(args->id != _CELL) {
	     return (CELLP)error(NEA);
	}
	if(args->car->id != _CELL) {
		return (CELLP)t;
	}
	return (CELLP)nil;
}

CELLP numberp_f(CELLP args)
{
	if(args->id != _CELL) {
		return (CELLP)error(NEA);
	}
	if(args->car->id == _FIX || args->car->id == _FLT) {
		return (CELLP)t;
	}
	return (CELLP)nil;
}

CELLP equal_f(CELLP args) 
{
	int equal();

	if(args->id != _CELL || args->cdr->id != _CELL) {
	     return (CELLP)error(NEA);
	}
	while(args->cdr->id == _CELL) {
		if(!equal(args->car, args->cdr->car)) {
			return (CELLP)nil;
		}
		args = args->cdr;
	}
	return (CELLP)t;
}

int equal(CELLP arg1, CELLP arg2) 
{
	if(arg1 == arg2) {
		return TRUE;
	}
	if(arg1->id != arg2->id) {
		return FALSE;
	}
	switch(arg1->id) {
		case _FIX:
			if(((NUMP)arg1)->value.fix == ((NUMP)arg2)->value.fix) {
				return TRUE;
			}
			else {
				return FALSE;
			}
		case _FLT:
			if(((NUMP)arg1)->value.flt == ((NUMP)arg2)->value.flt) {
				return TRUE;
			}
			else {
				return FALSE;
			}
		case _ATOM:
			if(strcmp(((ATOMP)arg1)->name, ((ATOMP)arg2)->name)) {
				return FALSE;
			}
			else {
				return TRUE;
			}
		case _CELL:
			if(equal(arg1->car, arg2->car) && equal(arg1->cdr, arg2->cdr)) {
				return TRUE;
			}
			else {
				return FALSE;
			}
		default:
			return (int)error(ULO);
	}
}

CELLP putprop_f(CELLP args)
{
	CELLP val, cp;
	ATOMP key,ap;
	int q;//N//

	if(args->id != _CELL
		|| args->cdr->id != _CELL
		|| args->cdr->cdr->id != _CELL) {
	     return (CELLP)error(NEA);
	}
	if((ap = (ATOMP)args->car)->id != _ATOM
		|| (key  = (ATOMP)args->cdr->cdr->car)->id != _ATOM) {
	     return (CELLP)error(IAA);
	}
	val = args->cdr->car;
	cp = ap->plist;
	for(cp = ap->plist; cp->id == _CELL; cp = cp->cdr->cdr) {
		if((ATOMP)cp->car == key) {
			return (cp->cdr->car = val);
		}
	}
	stackcheck;
	q = on(&args);//N//
	on(&val);//N//
	on(&cp);//N//
	on((CELLP *)&key);
	on((CELLP*)&ap);//N//
	*++sp = newcell(); ec;
	cp = *sp;
	//on(&cp);//N//
	cp->car = (CELLP)key;
	cp->cdr = newcell(); ec;
	off(q);
	cp->cdr->car = val;
	cp->cdr->cdr = ap->plist;
	ap->plist = *sp--;
	return val;
}

CELLP get_f(CELLP args) 
{
	CELLP cp;
	ATOMP key,ap;

	if(args->id != _CELL || args->cdr->id != _CELL) {
	     return (CELLP)error(NEA);
	}
	if((ap = (ATOMP)args->car)->id != _ATOM ||
				(key = (ATOMP)args->cdr->car)->id != _ATOM) {
	     return (CELLP)error(IAA);
	}
	for(cp = ap->plist; cp->id == _CELL; cp = cp->cdr->cdr) {
		if((ATOMP)cp->car == key) {
			return cp->cdr->car;
		}
	}

	return (CELLP)nil;
}

CELLP intern_f(CELLP arg)
{
  STR str = arg->car; // こんなふうにできる？
  ATOMP ap;
  print_s(str,ESCON);
  if((ap = old_atom(str)) == NULL)
  {
    int q = on(&arg);
    ap = mk_atom(str);
    off(q);
  }
  return (CELLP)ap;
}


CELLP remprop_f(CELLP args)
{
	CELLP val, cp;
	ATOMP key, ap;

	if(args->id != _CELL || args->cdr->id != _CELL) {
	     return (CELLP)error(NEA);
	}
	if((ap = (ATOMP)args->car)->id != _ATOM ||
			(key = (ATOMP)args->cdr->car)->id != _ATOM) {
	     return (CELLP)error(IAA);
	}
	if((cp = ap->plist) == (CELLP)nil) {
		return (CELLP)nil;
	}
	if((ATOMP)cp->car == key) {
		ap->plist = cp->cdr->cdr;
		return cp->cdr->car;
	}
	for(cp = cp->cdr; cp->cdr->id == _CELL; cp = cp->cdr->cdr) {
		if((ATOMP)cp->cdr->car == key) {
			val = cp->cdr->cdr->car;
			cp->cdr = cp->cdr->cdr->cdr;
			return val;
		}
	}
	return (CELLP)nil;
}

CELLP generate_atom_f(CELLP arg)
{
  ATOMP result;
  uchar name[NAMLEN + 1];
  int i;
  CELLP cp;
  char id;
  for(i = 0, cp = arg->car; cp != (CELLP)nil; ++i, cp = cp->cdr)
  {
		//とりあえずATOMとして確保しておく
    ATOMP ap = (ATOMP)(cp->car);
    id = cp->car->id;

    if(i >= NAMLEN)
    {
      return (CELLP)nil; // 長すぎるError
    }
    //でもやっぱりFIXでしたという場合は
    if(id == _FIX) {
	name[i] = ((NUMP)(cp->car))->value.fix + 48;
    }
    //ちゃんとATOMであった場合は
    else if(id == _ATOM) {
	name[i] = ap->name[0];
    }	
    else if(id != _ATOM)
    {
      return (CELLP)nil; // アトムか数値だけにしてくれError
    }
  }
  name[i] = NULL;
  if((result = old_atom(name)) == NULL) {
    return (CELLP)mk_atom(name);
  }
  return (CELLP)result;
}

static void defsubr(STR name, CELLP (*funcp)(), char type)//N//
{
	ATOMP ap, mk_atom();
	ap = mk_atom(name); //ec;//N//
	ap->ftype = type;
	ap->fptr = (CELLP)funcp;
}

/*
void ini_subr()//N//
{
	CELLP car_f(),   cdr_f(),    cons_f();
	CELLP atom_f(),  eq_f(),     equal_f();
	CELLP quote_f(), de_f(),     cond_f();
	CELLP setq_f(),  oblist_f(), quit_f();
	CELLP putprop_f(), get_f(), remprop_f();
	CELLP read_f(), terpri_f();
	CELLP print_f(), prinl_f(), princ_f();
	CELLP minus_f(), plus_f();

	defsubr("car",	car_f,	_SUBR);
	defsubr("cdr",  cdr_f,  _SUBR);
	defsubr("cons",	cons_f,	_SUBR);
	defsubr("atom",	atom_f, _SUBR);
	defsubr("eq",	eq_f,	_SUBR);
	defsubr("equal",equal_f,	_SUBR);
	defsubr("quote", quote_f,	_FSUBR);
	defsubr("de", de_f,	_FSUBR);
	defsubr("cond", cond_f,	_FSUBR);
	defsubr("setq", setq_f, _FSUBR);
	defsubr("oblist", oblist_f,	_SUBR);
	defsubr("quit", quit_f,	_SUBR);
	defsubr("putprop", putprop_f,	_SUBR);
	defsubr("get", get_f,	_SUBR);
	defsubr("remprop", remprop_f,	_SUBR);
	defsubr("read", read_f,	_SUBR);
	defsubr("terpri", terpri_f,	_SUBR);
	defsubr("print", print_f,	_SUBR);
	defsubr("prinl", prinl_f,	_SUBR);
	defsubr("princ", princ_f,	_SUBR);
	defsubr("minus", minus_f,	_SUBR);
	defsubr("plus", plus_f,	_SUBR);
}
*/
