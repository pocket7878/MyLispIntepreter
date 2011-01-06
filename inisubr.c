#include "lisp.h"
#include "read.h"//N//
#include "save.h"

static void init0(void);//N//
static void init1(void);//N//
static void init2(void);
static void init3(void);

static void defsubr(STR name,CELLP (*funcp)(),char type);//N//

void inisubr(void)//N//
{
	init0();
	init1();
	init2();
	init3();
}

static void init0(void)//N//
{
	CELLP car_f(), cdr_f(), cons_f();
	CELLP atom_f(), eq_f(), equal_f();
	CELLP quote_f(), de_f(), cond_f();
	CELLP setq_f(), oblist_f(), quit_f();
	CELLP putprop_f(), get_f(), remprop_f();
	CELLP read_f(), terpri_f();
	CELLP print_f(), prinl_f(), princ_f();
	CELLP minus_f(), plus_f();

	defsubr("car",	car_f,	_SUBR);
	defsubr("cdr",	cdr_f, 	_SUBR);
	defsubr("cons",	cons_f,	_SUBR);
	defsubr("atom",	atom_f,	_SUBR);
	defsubr("eq",	eq_f,		_SUBR);
	defsubr("equal", equal_f,	_SUBR);
	defsubr("quote", quote_f,	_FSUBR);
	defsubr("de",	de_f,	_FSUBR);
	defsubr("cond", cond_f,	_FSUBR);
	defsubr("setq", setq_f,	_FSUBR);
	defsubr("oblist", oblist_f,	_SUBR);
	defsubr("quit", quit_f,	_SUBR);
	defsubr("putprop", putprop_f,	_SUBR);
	defsubr("get", get_f,	_SUBR);
	defsubr("remprop_f", remprop_f, _SUBR);
	defsubr("read", read_f, _SUBR);
	defsubr("terpri", terpri_f, _SUBR);
	defsubr("print", print_f, _SUBR);
	defsubr("prinl", prinl_f, _SUBR);
	defsubr("princ", princ_f, _SUBR);
	defsubr("minus", minus_f, _SUBR);
	defsubr("plus", plus_f, _SUBR);
}

static void init1(void)//N//
{
	CELLP reclaim_f(), verbos_f();
	defsubr("reclaim", reclaim_f, _SUBR);
	defsubr("verbos", verbos_f, _SUBR);
}

static void init2(void)
{
	CELLP progn_f(), throw_f(), catch_f();
	CELLP catch_f(), let_f(), lets_f();
	CELLP prog_f(), ret_f(), go_f();

	defsubr("progn", progn_f, _FSUBR);
	defsubr("throw", throw_f, _FSUBR);
	defsubr("catch", catch_f, _FSUBR);
	defsubr("catcherror", catch_f, _FSUBR);
	defsubr("let", let_f, _FSUBR);
	defsubr("let*", lets_f, _FSUBR);
	defsubr("prog", prog_f, _FSUBR);
	defsubr("return", ret_f, _SUBR);
	defsubr("go", go_f, _FSUBR);
}	

static void init3(void)
{
	CELLP eval_f(), rplacd_f(), lessp_f(), and_f(), or_f();

	defsubr("eval", eval_f, _FSUBR);
	defsubr("rplacd", rplacd_f, _SUBR);
	defsubr("lessp", lessp_f, _SUBR);
	defsubr("and", and_f, _FSUBR);
	defsubr("or", or_f, _FSUBR);
}

static void defsubr(STR name,CELLP (*funcp)(),char type)//N//
{
	ATOMP ap, mk_atom();
	ap = mk_atom(name); //ec;//N//
	ap->ftype = type;
	ap->fptr = (CELLP)funcp;
}
