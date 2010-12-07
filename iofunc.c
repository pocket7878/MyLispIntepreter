#include "lisp.h"
#include "save.h"

static CELLP prin(CELLP args, int mode) {
  CELLP cp, error();
  if(args->id != _CELL) {
    return error(NEA);
  }
  int q = on(&args);
  print_s(cp = args->car, mode);
  off(q);
  return cp;
}

CELLP read_f(CELLP args) {
  CELLP read_s();
  return read_s(TOP);
}

CELLP print_f(CELLP args) {
  CELLP prin(), result;
  int q = on(&args);
  result = prin(args, ESCON); ec;
  off(q);
  fputc('\n', cur_fpo);
  return result;
}

CELLP prinl_f(CELLP args) {
  CELLP prin();
  return prin(args, ESCON);
}

CELLP princ_f(CELLP args) {
  CELLP prin();
  return prin(args, ESCOFF);
}


CELLP terpri_f() {
  fputc('\n', cur_fpo);
  return (CELLP)nil;
}
