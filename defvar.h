#ifndef _DEFVAR_H_
#define _DEFVAR_H_

FILE	*cur_fpi,	*cur_fpo;
ATOMP	t, nil, lambda, eofread, prompt;
CELLP oblist[TABLESIZ];

CELLP fromcelltop, tocelltop, freecell,freecelltop;
ATOMP fromatomtop, toatomtop, freeatom,freeatomtop;
NUMP  fromnumtop, tonumtop, freenum,freenumtop;
STR   fromstrtop, tostrtop, newstr,freestrtop;

uchar oneline[LINESIZ];
STR	txtp;

int err, err_no;
CELLP *stacktop, *sp;
int verbos;

#endif
