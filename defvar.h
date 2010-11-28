#ifndef _DEFVAR_H_
#define _DEFVAR_H_

FILE	*cur_fpi,	*cur_fpo;
ATOMP	t, nil, lambda, eofread, prompt;
CELLP oblist[TABLESIZ];

CELLP fromcelltop, tocelltop, freecell,freecelltop;
CELLP old_freecell,old_freecelltop;

ATOMP fromatomtop, toatomtop, freeatom,freeatomtop;
ATOMP old_freeatom,old_freeatomtop;

NUMP  fromnumtop, tonumtop, freenum,freenumtop;
NUMP  old_freenum,old_freenumtop;
STR   fromstrtop, tostrtop, newstr,freestrtop;
STR   old_freestr, old_freestrtop;

uchar oneline[LINESIZ];
STR	txtp;

int err, err_no;
CELLP *stacktop, *sp;
int verbos;

#endif
