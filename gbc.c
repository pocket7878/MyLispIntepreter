#include "lisp.h"
#define forever for(;;)
static void initCellArea(CELLP from);
static void initAtomArea(ATOMP from);
static void initNumArea(NUMP from);
static void Copying(CELLP *top, int n, int a);
static CELLP Copy(CELLP cp, int n, int a);

CELLP newcell() 
{
	CELLP cp;
	if((freecelltop + 1) > fromcelltop + (CELLSIZ / 2)) {
		gbc(OFF, ON);
		if((freecelltop + 1) > fromcelltop + (CELLSIZ / 2)) {
			fprintf(stdout,"Allocation failed!!!!!!\n");
			exit(0);
		}
	}
	cp = freecelltop;
	freecelltop++;
	cp->cdr = (CELLP)nil;
	return cp;
}

ATOMP newatom() {
	ATOMP ap;
	//新しいATOMPを用意すると半分からはみ出る場合にはGC起動
	if((freeatomtop + 1) > fromatomtop + (ATOMSIZ / 2)) {
		gbc(OFF, ON);
		if((freeatomtop + 1) > fromatomtop + (ATOMSIZ / 2)) {
			fprintf(stdout,"Allocation failed!!!!!!\n");
			exit(0);
		}
	}
	ap = freeatomtop;
	freeatomtop++;
	return ap;
}

NUMP newnum() {
	NUMP np;
	//新しいNUMを追加すると分からはみ出る場合にはGCを起動する。
	if((freenumtop + 1) > fromnumtop + (NUMSIZ / 2)){
		gbc(ON, OFF); ec;
		if((freenumtop + 1) > fromnumtop + (NUMSIZ / 2)) {
			fprintf(stdout,"Allocation failed!!!!!!\n");
			exit(0);
		}
	}
	np = freenumtop;
	freenumtop++;
	return np;
}

void gbc(int n, int a) {
	CELLP *sp1;
	CELLP Celltmp;
	ATOMP Atomtmp;
	NUMP Numtmp;
	int i, s;
	if(verbos) {
		if(n & a) {
			fprintf(stdout, "\nYou surprised GBC,\n");
		}
		else {
			fprintf(stdout, "\nGBC surprised You,\n");
		}
	}
	//Cellは標準でGCの対象である
	initCellArea(tocelltop);
	freecelltop = tocelltop;
	if(n) {
		initNumArea(tonumtop);
		freenumtop = tonumtop;
	}
	if(a) {
		initAtomArea(toatomtop);
		freeatomtop = toatomtop;
	}
	
	
	for(i = 0; i < TABLESIZ; ++i) {
		Copying(&oblist[i], n, a);
	}
	for(sp1 = stacktop; sp1 <= sp; ++sp1) {
		//Copying(*sp1, n, a);
	}
	//fromcelltopとtocelltopを交換する(DefaultでGCの対象なので)
	Celltmp = fromcelltop;
	fromcelltop = tocelltop;
	tocelltop = Celltmp;
	//TODO
	//From領域の初期化
	if(n) {
		//fromnumtopとtonumtopを交換する
		Numtmp = fromnumtop;
		fromnumtop = tonumtop;
		tonumtop = Numtmp;
	}
	if(a) {
		//fromatomtopとtocelltopを交換する
		Atomtmp = fromatomtop;
		fromatomtop = toatomtop;
		toatomtop = Atomtmp;
	}
}

static void initCellArea(CELLP from)
{
	CELLP cp;
	printf("** Init Cell Area Start **\n");
	for(cp = from; cp < from + (CELLSIZ / 2); ++cp) {
	cp->id = _CELL;
		cp->cpflag = NOTCOPIED;
	cp->car = (CELLP)nil;
	cp->cdr = (CELLP)nil;
	}
	(--cp)->cdr = (CELLP)nil;
}


static void initAtomArea(ATOMP from)
{
	ATOMP ap;
	//atomの連結リストを作成
	//nilはcar,cdrともに自分自身を指し示すのでatomの先頭にあるnilは無視する(よって+1から始める)
	for(ap = from + 1; ap < from + (ATOMSIZ / 2); ++ap) {
		ap->id = _ATOM;
		ap->cpflag = NOTCOPIED;
		ap->plist = (CELLP)nil;
	}
	
	(--ap)->plist = (CELLP)nil;
}

static void initNumArea(NUMP from)
{
	NUMP np;
	//numの連結リストを作成
	for(np = from; np < from + (NUMSIZ / 2); ++np) {
		np->id = _FIX;
		np->cpflag = NOTCOPIED;
		np->value.ptr = (NUMP)nil;
	}
	(--np)->value.ptr = (NUMP)nil;
}
	
static void Copying(CELLP *top, int n, int a)
{
	//n = ONの時はnumもCopyする,
	//a = ONの時はatom,strもCopyする
	*top = Copy(*top, n, a);
}

static CELLP Copy(CELLP cp, int n, int a)
{
	char c = cp->id;
	//コピー先を表すポインター
	if(cp == (CELLP)nil) {
		return cp;
	}
	if(cp->cpflag & COPIED) {
		return cp->forwarding;
	}
	else {
		switch(c) {
			case _ATOM:
				if(a) {
					cp->forwarding = memcpy(freeatomtop, cp, sizeof(ATOM));
					freeatomtop++;
					cp->cpflag |= COPIED;
					((ATOMP)cp->forwarding)->value = Copy(((ATOMP)cp->forwarding)->value, n, a);
					((ATOMP)cp->forwarding)->plist = Copy(((ATOMP)cp->forwarding)->plist, n, a); 
					if(!((((ATOMP)cp->forwarding)->ftype) & NONMRK)) {
						((ATOMP)cp->forwarding)->fptr   = Copy(((ATOMP)cp->forwarding)->fptr, n, a);
					}
					//コピーした際はコピー先のアドレスを返す
					return cp->forwarding;
				}
				return cp;
			case _CELL:
				cp->forwarding = memcpy(freecelltop, cp, sizeof(CELL));
				freecelltop++;
				cp->cpflag |= COPIED;
				cp->forwarding->car = Copy(cp->forwarding->car, n, a);
				cp->forwarding->cdr = Copy(cp->forwarding->cdr, n, a);
				return cp->forwarding;
			case _FIX:
				//FALL THROUT
			case _FLT:
				if(n) {
					cp->forwarding = memcpy(freenumtop, cp, sizeof(NUM));
					freenumtop++;
					cp->cpflag |= COPIED;
					((NUMP)cp->forwarding)->value.ptr = Copy(((NUMP)cp->forwarding)->value.ptr, n, a);
					return cp->forwarding;
				}
				return cp;
			defalut:
				return (int)error(ULO);
		}
	}
}
