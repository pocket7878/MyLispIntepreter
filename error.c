/*
 *
 * 	ERROR
 * define error message
 * set error code
 * 	if error happend
 */

#include "lisp.h"

static STR err_msg[] = {
	"I don't know what happend",
	"String area used up",
	"Number area used up",
	"Atom area used up",
	"Cell area used up",
	"Unidenfied Lisp Object",
	"Pseudo S expression",
	"Ctrl charactor sneaks in",
	"Undefined function",
	"Illegal function form",
	"Not enough arguments",
	"Illegal argument--Atom required",
	"Illegal argument--Number required",
	"Illegal argument--List required",
	"Illegal arg --Atom or List required",
	"Illegal arg --Atom or Number required",
	"Illegal arg --List or Number required",
	"Illegal argument--Fix Num required",
	"Illegal argument--Float Num required",
	"Illegal structure",
	"Illegal association list",
	"Illegal property list",
	"Envrironment is an atom",
	"Envrironment has an atom",
	"Condition Clause mus be a List",
	"Unexpected EOF",
	"Can't Change Constant value",
	"Undentified error",
	"Software stack used up"
};

CELLP error(int code)
{
	err = ERR;
	err_no = code;
	return NULL;
}

void pri_err(CELLP form)
{
	fprintf(stderr, "\nOops!\n");
	fprintf(stderr, "Error No.%d : %s\n", err_no, err_msg[err_no]);
	fprintf(stderr, "At ");
	if((ATOMP)form == nil) {
		fprintf(stderr, "toplevel");
	}
	else {
		cur_fpo = stderr;
		print_s(form, ESCON);
	}
	fprintf(stderr, "\n\n");
	err = ERROK;
}
