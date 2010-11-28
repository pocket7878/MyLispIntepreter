#ifndef _ERROR_H_
#define _ERROR_H_
/*
 *
 * 	ERROR
 * define error message
 * set error code
 * 	if error happend
 */

#include "lisp.h"

extern STR err_msg[];
CELLP error(int code);
void pri_err(CELLP form);

#endif 
