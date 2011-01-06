#include "lisp.h"
#include "eval.h"
#include "fun.h"
#include "save.h"

#include "print.h"

CELLP rplacd_f(CELLP args);

// (rplacd left right)
CELLP rplacd_f(CELLP args)
{
	args->car->cdr = args->cdr->car;
	return args->car;
}
