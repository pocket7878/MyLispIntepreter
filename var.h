#ifndef _VAR_H_
#define _VAR_H_

extern FILE *cur_fpi, *cur_fpo;
extern	ATOMP	t,nil,lambda,eofread,prompt;
extern CELLP oblist[TABLESIZ];
extern CELLP fromcelltop, tocelltop,freecell,freecelltop;
extern CELLP old_freecell,old_freecelltop;

extern	ATOMP fromatomtop, toatomtop, freeatom,freeatomtop;
extern	ATOMP old_freeatom,old_freeatomtop;

extern NUMP fromnumtop, tonumtop, freenum,freenumtop;
extern NUMP old_freenum,old_freenumtop;

extern STR fromstrtop, tostrtop, newstr,freestrtop;
extern STR old_newstr,old_freestrtop;

extern CELLP C1, C2;
extern ATOMP A1, A2;
extern NUMP N1, N2;

extern CELLP* cellpp[CELLPPSIZ];
extern int cellpptop;

extern uchar oneline[LINESIZ];
extern STR txtp;
extern int err, err_no;
extern CELLP *stacktop, *sp;
extern int verbos;

extern int threshold;

//extern ATOMP freesysatomtop, freesysatoms;
extern ATOMP freesysatomtop, freesysatom;	// !!!N
extern int save_in_sys_atom;

extern CELLP throwlabel, throwval;
extern int gc_time;
#endif
