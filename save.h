#ifndef _SAVE_H_
#define _SAVE_H_

#include "lisp.h"
int on(CELLP *P);
void off(int i);
void gc_aux(int n, int a);
#endif
