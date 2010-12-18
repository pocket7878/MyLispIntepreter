#include <stdlib.h>//N//
#include <string.h>//N//
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
int col_cell(void);//N//
int col_num(void);//N//
int col_atom(void);//N//
int col_str(void);//N//
void gbc(int n, int a);
void old_gbc(int n, int a);

int verbose = 1;
const unsigned int syoushinn = 3;//N//
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
    fprintf(stderr, "newcell call GBC...");
    gbc(OFF, ON);
    if((freecelltop + 1) >= fromcelltop + (CELLSIZ / 2)) {
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
  //予約語のための領域確保の場合
  if(save_in_sys_atom == 1) {
	fprintf(stdout, "Allocation System Atom\n");
       if((freesysatomtop + 1) >= freesysatomtop + SYSATOMS) {
	    fprintf(stderr, "Error:: Can't generate systemAtom");
	    exit(0);
       }
       ap = freesysatomtop;
       freesysatomtop++;
       return ap;
  }
  //新しいATOMPを用意すると半分からはみ出る場合にはGC起動
  if((freeatomtop + 1) >= fromatomtop + (ATOMSIZ / 2)) {
    fprintf(stderr, "newatom call GBC...");
    gbc(OFF, ON);
    if((freeatomtop + 1) >= fromatomtop + (ATOMSIZ / 2)) {
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
  if((freenumtop + 1) >= fromnumtop + (NUMSIZ / 2)){
    fprintf(stderr, "newnum call GBC...");
    gbc(ON, OFF); ec;
    if((freenumtop + 1) >= fromnumtop + (NUMSIZ / 2)) {
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
	
  printf("Copying oblist start**\n");
  for(i = 0; i < TABLESIZ; ++i) {
    Copying(&oblist[i], n, a);
  }
  printf("Copying stack start**\n");
  for(sp1 = stacktop + 1; sp1 <= sp; ++sp1) {
printf("sp before copying: %p: ", sp1);print_s(*sp1, ESCOFF);printf("\n");
    Copying(sp1, n, a);
printf("sp after copying: %p: ", sp1);print_s(*sp1, ESCOFF);printf("\n");
  }
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
printf("旧世代領域セル(%p)", cp);
	if(! (cp->cpflag & COPIED))
	{
printf("からの深いGC");
		cp->cpflag |= COPIED;
		Copying(&(cp->car), n, a);
		Copying(&(cp->cdr), n, a);
	}
printf("\n");
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
      if(!(ISCELLP(cp) || ISATOMP((ATOMP)cp) || ISNUMP((NUMP)cp)))//N//
	{
	  fprintf(stderr, "![%p]car[%p]:", cp, cp->car);
	  //	Copying(&(cp->car), n, a);
	  fprintf(stderr, "![%p]cdr[%p]:", cp, cp->cdr);
	  //	Copying(&(cp->cdr), n, a);
	}
      switch(c) {
      case _ATOM:
	      if(a) {
		      ATOMP to;
		      cp->age++;
		      if(cp->age >= syoushinn) {
			      //cp = promote((CELLP)cp);
			      to = (ATOMP)promote((CELLP)cp);//promoteはコピー先のアドレスを返す
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
		      printf("p[%d] ", ((ATOMP)cp)->plist);
		      Copying(&(((ATOMP)cp)->plist), n, a);//N//
		      printf("f[%d] ", ((ATOMP)cp)->fptr);
		      Copying(&(((ATOMP)cp)->fptr), n, a);//N//
	      }//N//
	return cp;
      case _CELL:
	{ CELLP to;
	   cp->age++;
	   if(cp->age >= syoushinn) {
//		cp = promote((CELLP)cp);
		to = promote((CELLP)cp);//promoteはコピー先のアドレスを返す
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
		if(cp->age >= syoushinn) {
		     //cp = promote(cp);
		     to = (NUMP)promote(cp);//promoteはコピー先のアドレスを返す
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

     //それ以外は旧世代領域へのコピー先のアドレスを返す
  {  CELLP to;
     switch(c) {
     case _ATOM:
	  if(old_freeatomtop == (ATOMP)nil) {
	       //もし旧世代領域がいっぱいだったら
	       old_gbc(OFF,ON);
	       if(old_freeatomtop == (ATOMP)nil) {
		    fprintf(stdout,"Allocation failed!!!!!!\n");
		    exit(0);
	       }
	  }
	  //Save old_freeatomtop
	to = (CELLP)old_freeatomtop;
	//Renew old_freeatomtop
	old_freeatomtop = (ATOMP)old_freeatomtop->plist;
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
	  if(old_freecelltop == (CELLP)nil) {
	       old_gbc(OFF,ON);
	       if(old_freecelltop == (CELLP)nil) {
		    fprintf(stdout,"Allocation failed!!!!!!\n");
		    exit(0);
	       }
	  }
	  //Save old_freeatomtop
	  to = old_freecelltop;
	  //Renew old_freeatomtop
	  old_freecelltop = old_freecelltop->cdr;
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
	  if(old_freenumtop == (NUMP)nil) {
	       old_gbc(ON,OFF);
	       if(old_freenumtop == (NUMP)nil) {
		    fprintf(stdout,"Allocation failed!!!!!!\n");
		    exit(0);
	       }
	  }
	  //Save old_freenumtop
	  to = (CELLP)old_freenumtop;
	  //Renew old_freenumtop
	  old_freenumtop = old_freenumtop->value.ptr;
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
	       fprintf(stdout, "\nYou surprised OLDGBC,\n");
	  }
	  else {
	       fprintf(stdout, "\nOLDGBC surprised You,\n");
	  }
     }
     for(i = 0; i < TABLESIZ; ++i) {
	     fprintf(stdout, "\nMarking %d of oblist\n", i);
	  mark(oblist[i], n);
     }
     for(sp1 = stacktop; sp1 <= sp; ++sp1) {
	     fprintf(stdout, "\nMarking stack\n", i);
	  mark(*sp1, n);
     }
     fprintf(stdout, "Collectiong Cell Start\n");
     i = col_cell(); //ec;//N//
     fprintf(stdout, "Collectiong Cell end\n");
     if(n) {
	  n = col_num(); //ec;//N//
     }
     if(a) {
	  s = col_str();
	  a = col_atom(); //ec;//N//
     }
     else {
	  rem_mark_atom();
     }
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
}

void mark(CELLP cp, int n) {
	char c = cp->id;
	//旧世代領域でない物も対象外
	if(!(ISOLDCELLP(cp) || ISOLDATOMP((ATOMP)cp) || ISOLDNUMP((NUMP)cp))) {//N//
	     return;
	}
	//nilもノータッチ
	if(cp == (CELLP)nil) {
		return;
	}
	//既に処理済みの物も対象外
	if(c & USED) {
		return;
	}
	switch(c) {
		case _ATOM: 
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

void rem_mark_num(void) {//N//
	NUMP np;
	for(np = old_freenumtop; np < old_freenumtop + NUMSIZ; ++np) {
		np->id &= FREE;
	}
}

void rem_mark_atom(void) {//N//
	ATOMP ap;
	for(ap = old_freeatomtop; ap < old_freeatomtop + ATOMSIZ; ++ap) {
		ap->id = _ATOM;
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
			return (int)error(CELLUP);
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
	fprintf(stdout, "Collectiong cell Start\n");
		if(++np >= old_freenumtop + NUMSIZ) {
			rem_mark_atom();
			return (int)error(NUMUP);
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
			return (int)error(ATOMUP);
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

     
