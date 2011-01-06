#include <stdlib.h>//N//
#include <string.h>//N//
#include <time.h> //For gettimeofday
#include "lisp.h"
#include "error.h"//N//
#include "print.h"//N//
#include "save.h"
#define forever for(;;)
void initCellArea(CELLP from);
void initAtomArea(ATOMP from);
void initNumArea(NUMP from);
void Copying(CELLP *top, int n, int a);
CELLP Copy(CELLP cp, int n, int a);
CELLP promote(CELLP cp);
void mark(CELLP cp, int n);
void rem_mark_num(void);//N//
void rem_mark_atom(void);//N//
void rem_mark_cell(void);
int col_cell(void);//N//
int col_num(void);//N//
int col_atom(void);//N//
int col_str(void);//N//
void gbc(int n, int a);
void old_gbc(int n, int a);
unsigned int calc_new_threshold(unsigned int current_threshold,int copied, int promoted);

int verbose = 1;
//Counter of gbc happen
unsigned int counter_gbc = 0;
unsigned int counter_old_gbc = 0;
//Threshold value
unsigned int syoushinn_cell = 3;//N//
unsigned int syoushinn_atom = 3;
unsigned int syoushinn_num = 3;

//Desired value of promote ratio
const float desired_value_of_promote_ratio = 0.5; //Half of copied value wanted to be promoted

//Flag value for enable(or disable) update threshold value
int update_syousinn = 1; //0: don't update 1: update after gbc

//Accumlator value of gbc data
int copied_cell = 0;
int copied_atom = 0;
int copied_num = 0;

int promoted_cell = 0;
int promoted_atom = 0;
int promoted_num = 0;

//const unsigned int syoushinn = 99999;//N//

//cellpptop = 0;//N


#define ISCELLP(x) (fromcelltop <= (x) && (x) < fromcelltop + (CELLSIZ / 2))
#define ISATOMP(x) (fromatomtop <= (x) && (x) < fromatomtop + (ATOMSIZ / 2))
#define ISNUMP(x)  (fromnumtop  <= (x) && (x) < fromnumtop  + (NUMSIZ / 2))
#define ISOLDCELLP(x) (old_freecell <= (x) && (x) < old_freecell + CELLSIZ)
#define ISOLDATOMP(x) (old_freeatom <= (x) && (x) < old_freeatom + ATOMSIZ)
#define ISOLDNUMP(x) (old_freenum <= (x) && (x) < old_freenum + NUMSIZ)


CELLP newcell() 
{
	CELLP cp;
	if((freecelltop + 1) >= fromcelltop + (CELLSIZ / 2)) {
		fprintf(stderr, "====== Newcell call GBC. ======\n");
		gbc(OFF, ON);
		if((freecelltop + 1) >= fromcelltop + (CELLSIZ / 2)) {
			fprintf(stdout,"New Cell allocation failed!!!!!!\n");
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
	//予約語のための領域確保の場合
	if(save_in_sys_atom == 1) {
		fprintf(stdout, "Allocation System Atom\n");
		//       if((freesysatomtop + 1) >= freesysatomtop + SYSATOMS) {
		if((freesysatomtop + 1) >= freesysatom + SYSATOMS) {	// !!!N
			fprintf(stderr, "Error:: Can't generate systemAtom");
			exit(0);
		}
		ap = freesysatomtop;
		freesysatomtop++;
		return ap;
	}
	//新しいATOMPを用意すると半分からはみ出る場合にはGC起動
	if((freeatomtop + 1) >= fromatomtop + (ATOMSIZ / 2)) {
		fprintf(stderr, "====== New atom call GBC. ======\n");
		gbc(OFF, ON);
		if((freeatomtop + 1) >= fromatomtop + (ATOMSIZ / 2)) {
			fprintf(stdout,"New atom allocation failed!!!!!!\n");
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
		if((freenumtop + 1) >= fromnumtop + (NUMSIZ / 2)){
			fprintf(stderr, "====== New num call GBC. ======\n");
			gbc(ON, OFF); ec;
			if((freenumtop + 1) >= fromnumtop + (NUMSIZ / 2)) {
				fprintf(stdout,"New num allocation failed!!!!!!\n");
				exit(0);
			}
		}
		np = freenumtop;
		freenumtop++;
		return np;
	}


	void gbc(int n, int a) {
		FILE *fp;
		CELLP *sp1;
		CELLP Celltmp;
		ATOMP Atomtmp;
		NUMP Numtmp;
		int i, s;
		clock_t start_time, end_time;
		start_time = clock();
		counter_gbc++;
		if(verbos) {
			if(n & a) {
				fprintf(stdout, "[GC You surprised GC.]\n");
			}
			else {
				fprintf(stdout, "[GC GC surprised You.]\n");
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

		fprintf(stdout,"[GC Copying oblist start.]\n");
		for(i = 0; i < TABLESIZ; ++i) {
			Copying(&oblist[i], n, a);
		}
		fprintf(stdout,"[GC Copying oblist end.]\n");
		fprintf(stdout,"[GC Copying stack start.]\n");
		for(sp1 = stacktop + 1; sp1 <= sp; ++sp1) {
			//printf("sp before copying: %p: ", sp1);print_s(*sp1, ESCOFF);printf("\n");
			Copying(sp1, n, a);
			//printf("sp after copying: %p: ", sp1);print_s(*sp1, ESCOFF);printf("\n");
		}
		fprintf(stdout,"[GC Copying stack end.]\n");
		gc_aux(n, a);

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

		// 旧世代領域の、COPIEDフラグを下ろす（Copy() 冒頭のコメント参照）
		{
			ATOMP ap;
			for(ap = old_freeatom; ap < old_freeatom + ATOMSIZ; ++ap)
			{
				ap->cpflag = NOTCOPIED;
			}
		}
		{
			CELLP cp;
			for(cp = old_freecell; cp < old_freecell + CELLSIZ; ++cp)
			{
				cp->cpflag = NOTCOPIED;
			}
		}
		/* 今のところnumberに対しては不要
		   {
		   NUMP np;
		   for(np = old_freenum; np < old_freenum + NUMSIZ; ++np)
		   {
		   np->cpflag = NOTCOPIED;
		   }
		   }
		   */
		end_time = clock();
		fprintf(stdout, "==== GC information (CELL) ====\n");
		fprintf(stdout, "[GC time %10.10f\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);
		fprintf(stdout, "[%d cell was copied]\n",copied_cell);
		fprintf(stdout, "[%d cell was promoted]\n",promoted_cell);
		if(a) {
			fprintf(stdout, "==== GC information (ATOM) ====\n");
			fprintf(stdout, "[%d atom was copied]\n",copied_atom);
			fprintf(stdout, "[%d atom was promoted]\n",promoted_atom);
		}
		if(n) {
			fprintf(stdout, "==== GC information (NUM) ====\n");
			fprintf(stdout, "[%d num was copied]\n",copied_num);
			fprintf(stdout, "[%d num  was promoted]\n",promoted_num);
		}
		if((fp = fopen("gbc.log","a")) == NULL) {
			fprintf(stderr,"ERROR: gbc.log open error!!\n");
			exit(1);
		}
		fprintf(fp, "%d %10.10f\n",counter_gbc, (double)(end_time - start_time) / CLOCKS_PER_SEC);
		fclose(fp);
		if(update_syousinn == 1) {
			syoushinn_cell = calc_new_threshold(syoushinn_cell,copied_cell, promoted_cell);
			if(a) {
				syoushinn_atom = calc_new_threshold(syoushinn_atom,copied_atom, promoted_atom);
			}
			if(n) {
				syoushinn_num = calc_new_threshold(syoushinn_num,copied_num, promoted_num);
			}
		}
		copied_cell = 0;
		promoted_cell = 0;
		if(a) {
			copied_atom = 0;
			promoted_atom = 0;
		}
		if(n) {
			copied_num = 0;
			promoted_num = 0;
		}
	}

	unsigned int calc_new_threshold(unsigned int current_threshold,int copied, int promoted) {
		float current_ratio;
		if(copied == 0) {
			//Avoid division by zero!!
			return current_threshold;
		}
		current_ratio = (promoted / copied);
		if(current_ratio > desired_value_of_promote_ratio) {
			//TODO set more  authentic value. 
			return current_threshold + 1;
		}
		else if(current_ratio < desired_value_of_promote_ratio) {
			return current_threshold - 1;
		}
		return current_threshold;
	}

	void initCellArea(CELLP from)
	{
		CELLP cp;
		printf("** Init Cell Area Start **\n");
		for(cp = from; cp < from + (CELLSIZ / 2); ++cp) {
			cp->id = _CELL;
			cp->age = 0;
			cp->cpflag = NOTCOPIED;
			cp->forwarding = (CELLP)nil;
			cp->car = (CELLP)nil;
			cp->cdr = (CELLP)nil;
		}
		(--cp)->cdr = (CELLP)nil;
	}


	void initAtomArea(ATOMP from)
	{
		ATOMP ap;
		//atomの連結リストを作成
		//nilはcar,cdrともに自分自身を指し示すのでatomの先頭にあるnilは無視する(よって+1から始める)
		printf("** Init Atom Area Start **\n");
		for(ap = from + 1; ap < from + (ATOMSIZ / 2); ++ap) {
			ap->id = _ATOM;
			ap->age = 0;//N//
			ap->cpflag = NOTCOPIED;
			ap->forwarding = (CELLP)nil;//N//
			ap->value = (CELLP)nil;//N//
			ap->plist = (CELLP)nil;
			ap->name = "";//N//
			ap->ftype = 0;//N//
			ap->fptr = (CELLP)nil;//N//
		}

		(--ap)->plist = (CELLP)nil;
	}

	void initNumArea(NUMP from)
	{
		NUMP np;
		//numの連結リストを作成
		printf("** Init Num Area Start **\n");
		for(np = from; np < from + (NUMSIZ / 2); ++np) {
			np->id = _FIX;
			np->age = 0;
			np->cpflag = NOTCOPIED;
			np->value.ptr = (NUMP)nil;
		}
		(--np)->value.ptr = (NUMP)nil;
	}

	void Copying(CELLP *top, int n, int a)
	{
		//n = ONの時はnumもCopyする,
		//a = ONの時はatom,strもCopyする
		*top = Copy(*top, n, a);
	}

	CELLP Copy(CELLP cp, int n, int a)
	{
		//旧世代領域に居るやつ“自体”は無視するけど、そこから指されているやつは無視しちゃだめ
		// 一度処理した旧世代領域にいるやつにはCOPIEDフラグをたてて多重に処理してしまうことを防ぐ
		// 例	アトムfのfptrに (lambda (x) (... (f (plus x -1)) ...)) がはいっていて、fが旧世代
		//	領域にあるとき、fptrのCopyingでふたたびfをCopyすることになる
		// gbc() の最後に、旧世代領域のすべてのcpflagからCOPIEDフラグを下ろす処理を加えている
		if(ISOLDATOMP((ATOMP)cp))
		{
			if(! (cp->cpflag & COPIED))
			{
				cp->cpflag |= COPIED;
				Copying(&(((ATOMP)cp)->plist), n, a);
				Copying(&(((ATOMP)cp)->fptr), n, a);
			}
			return cp;
		}
		else if(ISOLDCELLP(cp))
		{
			//printf("旧世代領域セル(%p)", cp);
			if(! (cp->cpflag & COPIED))
			{
				//printf("からの深いGC");
				cp->cpflag |= COPIED;
				Copying(&(cp->car), n, a);
				Copying(&(cp->cdr), n, a);
			}
			//printf("\n");
			return cp;
		}
		else if(ISOLDNUMP((NUMP)cp)) // numberから指されるものはいまのところない
		{
			return cp;
		}
		//char c = cp->id;//N//
		//コピー先を表すポインター
		if(!(ISCELLP(cp) || ISATOMP((ATOMP)cp) || ISNUMP((NUMP)cp))) {//N//
			return cp;			
		}
		else {
			if(cp == (CELLP)nil) {
				return cp;
			}
			if(cp->cpflag & COPIED) {
				return cp->forwarding;
			}
			else {
				char c = cp->id;//N//
				switch(c) {
					case _ATOM:
						if(a) {
							ATOMP to;
							cp->age++;
							copied_atom++;
							if(cp->age >= syoushinn_atom) {
								//cp = promote((CELLP)cp);
								to = (ATOMP)promote((CELLP)cp);//promoteはコピー先のアドレスを返す
								promoted_atom++;
							} else {
								to = freeatomtop++;
							}
							cp->forwarding = memcpy(to, cp, sizeof(ATOM));
							cp->cpflag |= COPIED;
							((ATOMP)cp->forwarding)->value = Copy(((ATOMP)cp->forwarding)->value, n, a);
							((ATOMP)cp->forwarding)->plist = Copy(((ATOMP)cp->forwarding)->plist, n, a); 
							if(!((((ATOMP)cp->forwarding)->ftype) & NONMRK)) {
								((ATOMP)cp->forwarding)->fptr   = Copy(((ATOMP)cp->forwarding)->fptr, n, a);
							}
							//コピーした際はコピー先のアドレスを返す
							return cp->forwarding;
						}
						else//N//
						{//N//
							//		      printf("p[%d] ", ((ATOMP)cp)->plist);
							Copying(&(((ATOMP)cp)->plist), n, a);//N//
							//		      printf("f[%d] ", ((ATOMP)cp)->fptr);
							Copying(&(((ATOMP)cp)->fptr), n, a);//N//
						}//N//
						return cp;
					case _CELL:
						{ CELLP to;
							cp->age++;
							copied_cell++;
							if(cp->age >= syoushinn_cell) {
								//		cp = promote((CELLP)cp);
								to = promote((CELLP)cp);//promoteはコピー先のアドレスを返す
								promoted_cell++;
							} else {
								to = freecelltop++;
							}
							cp->forwarding = memcpy(to, cp, sizeof(CELL));
							cp->cpflag |= COPIED;
							cp->forwarding->car = Copy(cp->forwarding->car, n, a);
							cp->forwarding->cdr = Copy(cp->forwarding->cdr, n, a);
							return cp->forwarding;
						}
					case _FIX:
						//FALL THROUT
					case _FLT:
						if(n) {
							NUMP to;
							cp->age++;
							copied_num++;
							if(cp->age >= syoushinn_num) {
								//cp = promote(cp);
								to = (NUMP)promote(cp);//promoteはコピー先のアドレスを返す
								promoted_num++;
							} else {
								to = freenumtop++;
							}
							cp->forwarding = memcpy(to, cp, sizeof(NUM));
							cp->cpflag |= COPIED;
							return cp->forwarding;
						}
						return cp;
					default://N//
						return (CELLP)error(ULO);//N//
				}
			}
		}
	}

	// コピー先のアドレスを返すだけ
	// コピーなど必要な処理は呼び出し元（Copy() 側で行う）
	CELLP promote(CELLP cp) {
		char c = cp->id;//N//
		//すでに旧世代領域に存在したら何もしない
		if(ISOLDCELLP(cp) || ISOLDATOMP((ATOMP)cp) || ISOLDNUMP((NUMP)cp)) {//N//
			return cp;
		}
		//CELLでもATOMでもNUMでも無い物はノータッチ
		if (!(ISCELLP(cp) || ISATOMP((ATOMP)cp) || ISNUMP((NUMP)cp))) {//N//
			return cp;
		}

		printf("promote: %p (%02x)[Age: %d]\n", cp, (int)c,cp->age);
		//それ以外は旧世代領域へのコピー先のアドレスを返す
		{  CELLP to;
			switch(c) {
				case _ATOM:
					if(old_freeatom == (ATOMP)nil) {
						//もし旧世代領域がいっぱいだったら
						fprintf(stdout, "====== Promote call OLDGC ======\n");
						old_gbc(OFF,ON);
						if(old_freeatom == (ATOMP)nil) {
							fprintf(stdout,"OLD: New atom allocation failed!!!!!!\n");
							exit(0);
						}
					}
					//Save old_freeatomtop
					to = (CELLP)old_freeatom;
					//Renew old_freeatomtop
					old_freeatom = (ATOMP)old_freeatom->plist;
					//Shallow copy
					//	*((ATOMP)tmp) = *((ATOMP)cp);
					/*
					   (ATOMP)tmp->id = (ATOMP)cp->id;
					   (ATOMP)tmp->age = (ATOMP)cp->age;
					   (ATOMP)tmp->cpflag = (ATOMP)cp->cpflag;
					   (ATOMP)tmp->forwarding = (ATOMP)cp->forwarding;
					   (ATOMP)tmp->value = (ATOMP)cp->value;
					   (ATOMP)tmp->plist = (ATOMP)cp->plist;
					   (ATOMP)tmp->name = (ATOMP)cp->name;
					   (ATOMP)tmp->ftype = (ATOMP)cp->ftype;
					   (ATOMP)tmp->fptr = (ATOMP)cp->fptr;
					   */
					//	cp->cpflag |= COPIED;
					//	cp->forwarding = tmp;
					//old_freeatomtop++;
					break;
				case _CELL:
					if(old_freecell == (CELLP)nil) {
						fprintf(stdout, "====== Promote call OLDGC ======\n");
						old_gbc(OFF,ON);
						if(old_freecell == (CELLP)nil) {
							fprintf(stdout,"OLD: New cell allocation failed!!!!!!\n");
							exit(0);
						}
					}
					//Save old_freeatomtop
					to = old_freecell;
					//Renew old_freeatomtop
					old_freecell = old_freecell->cdr;
					//Shallow copy
					//	  *tmp = *cp;
					/*
					   tmp->id = cp->id;
					   tmp->age = cp->age;
					   tmp->cpflag = cp->cpflag;
					   tmp->forwarding = cp->forwarding;
					   tmp->car = cp->car;
					   tmp->cdr = cp->cdr;
					   */
					//	  cp->cpflag |= COPIED;
					//	  cp->forwarding = tmp;
					//	  old_freecelltop++;
					break;
				case _FIX:
					//FALL THROUT
				case _FLT:
					if(old_freenum == (NUMP)nil) {
						fprintf(stdout, "====== Promote call OLDGC ======\n");
						old_gbc(ON,OFF);
						if(old_freenum == (NUMP)nil) {
							fprintf(stdout,"OLD: New num allocation failed!!!!!!\n");
							exit(0);
						}
					}
					//Save old_freenumtop
					to = (CELLP)old_freenum;
					//Renew old_freenumtop
					old_freenum = old_freenum->value.ptr;
					//Shallow copy
					//	  *((NUMP)tmp) = *((NUMP)cp);
					/*
					   (NUMP)tmp->id = (NUMP)cp->id;
					   (NUMP)tmp->age = (NUMP)cp->age;
					   (NUMP)tmp->cpflag = (NUMP)cp->cpflag;
					   (NUMP)tmp->forwarding = (NUMP)cp->forwarding;
					   (NUMP)tmp->value.ptr = (NUMP)cp->value.ptr;
					   (NUMP)tmp->value.fix = (NUMP)cp->value.fix;
					   (NUMP)tmp->value.flt = (NUMP)cp->value.flt;
					   */
					//	  cp->cpflag |= COPIED;
					//	  cp->forwarding = tmp;
					//old_freenumtop++;
					break;
			}
			//return cp;
			return to;//コピー先のアドレスを返す
		}
	}

	void old_gbc(int n, int a) {
		int i, s;
		CELLP *sp1;
		if(verbose) {
			if(n & a) {
				fprintf(stdout, "\n[You surprised OLDGBC.]\n");
			}
			else {
				fprintf(stdout, "\n[OLDGBC surprised You.]\n");
			}
		}

		fprintf(stdout, "[OLDGC Marking oblist start.]\n");
		for(i = 0; i < TABLESIZ; ++i) {
			mark(oblist[i], n);
		}
		fprintf(stdout, "[OLDGC Marking oblist end.]\n");

		fprintf(stdout, "[OLDGC Marking stack start.]\n");
		for(sp1 = stacktop; sp1 <= sp; ++sp1) {
			mark(*sp1, n);
		}
		fprintf(stdout, "[OLDGC Marking stack end.]\n");

		fprintf(stdout, "[OLDGC Marking AUX start.]\n");
		old_gc_aux(n);
		fprintf(stdout, "[OLDGC Marking AUX end.]\n");

		fprintf(stdout, "[OLDGC Collectiong Cell start.]\n");
		i = col_cell(); //ec;//N//
		fprintf(stdout, "[OLDGC Collectiong Cell end.]\n");
		rem_mark_cell();

		if(n) {
			fprintf(stdout, "[OLDGC Collectiong Num start.]\n");
			n = col_num(); //ec;//N//
			fprintf(stdout, "[OLDGC Collectiong Num end.]\n");
		}
		rem_mark_num();

		if(a) {
			//     fprintf(stdout, "[OLDGC Collectiong Str start.]\n");
			//	  s = col_str();
			//     fprintf(stdout, "[OLDGC Collectiong Str end.]\n");
			fprintf(stdout, "[OLDGC Collectiong Atom start.]\n");
			a = col_atom(); //ec;//N//
			fprintf(stdout, "[OLDGC Collectiong Atom end.]\n");
		}
		rem_mark_atom();

		if(verbose) {
			fprintf(stdout, "\tfree cell %d\n", i);
			if(n) {
				fprintf(stdout, "\tfree num = %d\n", n);
			}
			if(a) {
				fprintf(stdout, "\tfree atom = %d\n", a);
				fprintf(stdout, "\tfree str = %d\n", s);
			}
		}
		fprintf(stdout,"[OLDGC Finished]\n");

		return;
	}

	/*
	   void mark(CELLP cp, int n) {
	   char c = cp->id;
//既に処理済みの物は対象外
if(c & USED) {
return;
}
//nilもノータッチ
if(cp == (CELLP)nil) {
return;
}
//旧世代領域でない物も対象外だが
if(!(ISOLDCELLP(cp) || ISOLDATOMP((ATOMP)cp) || ISOLDNUMP((NUMP)cp))) {//N//
//接続されているものが旧世代領域である場合も考慮する
switch(c) {
case _ATOM: 
printf("[mark not old atom ");print_s(cp, ESCOFF);printf("]\n");
cp->id |= USED;
mark(((ATOMP)cp)->value, n);
mark(((ATOMP)cp)->plist, n);
if(((ATOMP)cp)->forwarding) mark(((ATOMP)cp)->forwarding, n);
if(!((((ATOMP)cp)->ftype) & NONMRK)) {
mark(((ATOMP)cp)->fptr, n);
}
break;
case _CELL: 
cp->id |= USED;
if(cp->forwarding) mark(cp->forwarding, n);
mark(cp->car, n);
mark(cp->cdr, n);
break;
case _FIX:
case _FLT:
if(n) {
cp->id |= USED;
if(((NUMP)cp)->forwarding) mark(((NUMP)cp)->forwarding, n);
}
break;
}
return;
}
if(ISOLDCELLP(cp) || ISOLDATOMP((ATOMP)cp) || ISOLDNUMP((NUMP)cp))
{
	// 旧世代領域
	switch(c) {
	case _ATOM: 
	printf("[mark old atom ");print_s(cp, ESCOFF);printf("]\n");
	cp->id |= USED;
	mark(((ATOMP)cp)->value, n);
	mark(((ATOMP)cp)->plist, n);
	if(!((((ATOMP)cp)->ftype) & NONMRK)) {
	mark(((ATOMP)cp)->fptr, n);
	}
	break;
	case _CELL: 
	cp->id |= USED;
	mark(cp->car, n);
	mark(cp->cdr, n);
	break;
	case _FIX:
	case _FLT:
	if(n) {
	cp->id |= USED;
	}
	break;
//default://N//
//return (int)error(ULO);//N//
}
}
return;
}
*/


void mark(CELLP cp, int n) {
	char c = cp->id;
	//既に処理済みの物は対象外
	if(c & USED) {
		return;
	}
	//nilもノータッチ
	if(cp == (CELLP)nil) {
		return;
	}

	// 以下は、atom、cell、numの新世代の2つの領域と旧世代領域、sysatom領域について同じ処理
	switch(c) {
		case _ATOM: 
			//if(prompt == (ATOMP)cp) { printf("[M prompt's id=%x] --> ", (int)(((ATOMP)cp)->id)); }
			cp->id |= USED;
			//if(prompt == (ATOMP)cp) { printf("[M prompt's id=%x]\n", (int)(((ATOMP)cp)->id)); }
			mark(((ATOMP)cp)->value, n);
			mark(((ATOMP)cp)->plist, n);
			mark(((ATOMP)cp)->forwarding, n);
			if(!((((ATOMP)cp)->ftype) & NONMRK)) {
				mark(((ATOMP)cp)->fptr, n);
			}
			break;
		case _CELL: 
			cp->id |= USED;
			mark(cp->forwarding, n);
			mark(cp->car, n);
			mark(cp->cdr, n);
			break;
		case _FIX:
		case _FLT:
			if(n) {
				cp->id |= USED;
				if(((NUMP)cp)->forwarding) mark(((NUMP)cp)->forwarding, n);
			}
			break;
	}
	return;
}

void rem_mark_num(void) {//N//
	NUMP np;
	for(np = fromnumtop; np < fromnumtop + (NUMSIZ/2); ++np) {
		np->id &= FREE;
	}
	for(np = tonumtop; np < tonumtop + (NUMSIZ/2); ++np) {
		np->id &= FREE;
	}
	for(np = old_freenumtop; np < old_freenumtop + NUMSIZ; ++np) {
		np->id &= FREE;
	}
}

void rem_mark_atom(void) {//N//
	ATOMP ap;
	for(ap = fromatomtop; ap < fromatomtop + (ATOMSIZ/2); ++ap) {
		//if(prompt == ap) { printf("[F prompt's id=%x] --> ", (int)ap->id); }
		ap->id = _ATOM;
		//if(prompt == ap) { printf("[F prompt's id=%x]\n", (int)ap->id); }
	}
	for(ap = toatomtop; ap < toatomtop + (ATOMSIZ/2); ++ap) {
		//if(prompt == ap) { printf("[T prompt's id=%x] --> ", (int)ap->id); }
		ap->id = _ATOM;
		//if(prompt == ap) { printf("[T prompt's id=%x]\n", (int)ap->id); }
	}
	for(ap = old_freeatomtop; ap < old_freeatomtop + ATOMSIZ; ++ap) {
		//if(prompt == ap) { printf("[O prompt's id=%x] --> ", (int)ap->id); }
		ap->id = _ATOM;
		//if(prompt == ap) { printf("[O prompt's id=%x]\n", (int)ap->id); }
	}
	//	for(ap = freesysatomtop; ap < freesysatomtop + SYSATOMS; ++ap) {
	for(ap = freesysatom; ap < freesysatom + SYSATOMS; ++ap) {	// !!!N
		//if(prompt == ap) { printf("[S prompt's id=%x] --> ", (int)ap->id); }
		ap->id = _ATOM;
		//if(prompt == ap) { printf("[S prompt's id=%x]\n", (int)ap->id); }
	}
}

void rem_mark_cell(void) {
	//新世代領域の中でもマークされるものが発生するので
	CELLP cp;
	for(cp = fromcelltop; cp < fromcelltop + (CELLSIZ/2); ++cp) {
		cp->id = _CELL;
	}
	for(cp = tocelltop; cp < tocelltop + (CELLSIZ/2); ++cp) {
		cp->id = _CELL;
	}
	//	for(cp = freecelltop; cp < freecelltop + CELLSIZ; ++cp) {
	for(cp = old_freecelltop; cp < old_freecelltop + CELLSIZ; ++cp) {
		cp->id = _CELL;
	}
}
int col_cell(void) //N//
{
	int n = 1;
	CELLP end;
	CELLP cp = old_freecelltop;
	while(cp->id & USED) {
		cp->id &= FREE;
		if(++cp >= old_freecelltop + CELLSIZ) {
			rem_mark_num();
			rem_mark_atom();
			//return (int)error(CELLUP);
			old_freecell = (CELLP)nil;
			return 0;
		}
	}
	old_freecell = end = cp++;
	end->car = (CELLP)nil;
	for(; cp < old_freecelltop + CELLSIZ; ++cp) {
		if(cp->id & USED) {
			cp->id &= FREE;
			continue;
		}
		end->cdr = cp;
		end = cp;
		end->car = (CELLP)nil;
		++n;
	}
	end->cdr = (CELLP)nil;
	return n;
}

int col_num(void) {//N//
	int n = 1;
	NUMP end, np = old_freenumtop;
	forever {
		if(!(np->id & USED)) {
			break;
		}
		np->id &=FREE;
		if(++np >= old_freenumtop + NUMSIZ) {
			rem_mark_atom();
			//return (int)error(NUMUP);
			old_freenum = (NUMP)nil;
			return 0;
		}
	}
	old_freenum = end = np++;
	end->id = _FIX;
	for(; np < old_freenumtop + NUMSIZ; ++np) {
		if(np->id & USED) {
			np->id &= FREE;
			continue;
		}
		end->value.ptr = np;
		end = np;
		end->id = _FIX;
		++n;
	}
	end->value.ptr = (NUMP)nil;
	return n;
}

int col_atom(void) {//N//
	int n = 1;
	ATOMP end, ap = old_freeatomtop;
	forever {
		if(!(ap->id & USED)) {
			break;
		}
		ap->id &= FREE;
		if(++ap >= old_freeatomtop + ATOMSIZ) {
			//return (int)error(ATOMUP);
			old_freeatom = nil;
			return 0;
		}
	}
	old_freeatom = end = ap++;
	for(; ap < old_freeatomtop + ATOMSIZ; ++ap) {
		if(ap->id & USED) {
			ap->id &= FREE;
			continue;
		}
		end->plist = (CELLP)ap;
		end = ap;
		++n;
	}
	end->plist = (CELLP)nil;
	return n;
}

int col_str(void)//N//
{
	STR s, end;
	ATOMP ap;
	*newstr = '\0';
	for(s = end = freestrtop + strlen("nil")+1; s < newstr;) {
		for(ap = old_freeatomtop + 1; ap < old_freeatomtop + ATOMSIZ; ++ap) {
			if(!(ap->id & USED)) {
				continue;
			}
			if(ap->name == s) {
				if(end != s) {
					strcpy(end, s);
					ap->name = end;
				}
				while(*end++ != '\0');
				break;
			}
		}
		while(*s++ != '\0');
		while(*s == '\0' && s < newstr) {
			++s;
		}
	}
	newstr = end;
	return (int)((old_freestrtop + STRSIZ) - newstr);
}


