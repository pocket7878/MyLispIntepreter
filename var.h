#ifndef _VAR_H_
#define _VAR_H_

extern FILE *cur_fpi, *cur_fpo;
extern	ATOMP	t,nil,lambda,eofread,prompt;
extern CELLP oblist[TABLESIZ];
extern CELLP fromcelltop, tocelltop,freecell,freecelltop;
extern	ATOMP fromatomtop, toatomtop, freeatom,freeatomtop;
extern NUMP fromnumtop, tonumtop, freenum,freenumtop;
extern STR fromstrtop, tostrtop, newstr,freestrtop;

extern uchar oneline[LINESIZ];
extern STR txtp;
extern int err, err_no;
extern CELLP *stacktop, *sp;
extern int verbos;

#endif
