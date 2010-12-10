#include "lisp.h"
#include "save.h"
#define forever for(;;)
void initCellArea(CELLP from);
void initAtomArea(ATOMP from);
void initNumArea(NUMP from);
void Copying(CELLP *top, int n, int a);
CELLP Copy(CELLP cp, int n, int a);
CELLP promote(CELLP cp);
void mark(CELLP cp, int n);
void rem_mark_num();
void rem_mark_atom();
int col_cell();
int col_num();
int col_atom();
int col_str();
void gbc(int n, int a);
void old_gbc(int n, int a);

int verbose = 1;
int syoushinn = 3;

cellpptop = 0;

#define ISCELLP(x) (fromcelltop <= (x) && (x) < fromcelltop + (CELLSIZ / 2))
#define ISATOMP(x) (fromatomtop <= (x) && (x) < fromatomtop + (ATOMSIZ / 2))
#define ISNUMP(x)  (fromnumtop  <= (x) && (x) < fromnumtop  + (NUMSIZ / 2))
#define ISOLDCELLP(x) (old_freecell <= (x) && (x) < old_freecell + CELLSIZ)
#define ISOLDATOMP(x) (old_freeatom <= (x) && (x) < old_freeatom + ATOMSIZ)
#define ISOLDNUMP(x) (old_freenum <= (x) && (x) < old_freenum + NUMSIZ)


CELLP newcell() 
{
  CELLP cp;
  if((freecelltop + 1) > fromcelltop + (CELLSIZ / 2)) {
    fprintf(stderr, "newcell call GBC...");
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
  //予約語のための領域確保の場合
  if(save_in_sys_atom == 1) {
	fprintf(stdout, "Allocation System Atom\n");
       if((freesysatomtop + 1) > freesysatomtop + SYSATOMS) {
	    fprintf(stderr, "Error:: Can't generate systemAtom");
	    exit(0);
       }
       ap = freesysatomtop;
       freesysatomtop++;
       return ap;
  }
  //新しいATOMPを用意すると半分からはみ出る場合にはGC起動
  if((freeatomtop + 1) > fromatomtop + (ATOMSIZ / 2)) {
    fprintf(stderr, "newatom call GBC...");
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
    fprintf(stderr, "newnum call GBC...");
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
	
  printf("Copying oblist start**\n");
  for(i = 0; i < TABLESIZ; ++i) {
    Copying(&oblist[i], n, a);
  }
  printf("Copying stack start**\n");
  for(sp1 = stacktop + 1; sp1 <= sp; ++sp1) {
    Copying(sp1, n, a);
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
}

void initCellArea(CELLP from)
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


void initAtomArea(ATOMP from)
{
  ATOMP ap;
  //atomの連結リストを作成
  //nilはcar,cdrともに自分自身を指し示すのでatomの先頭にあるnilは無視する(よって+1から始める)
  printf("** Init Atom Area Start **\n");
  for(ap = from + 1; ap < from + (ATOMSIZ / 2); ++ap) {
    ap->id = _ATOM;
    ap->cpflag = NOTCOPIED;
    ap->plist = (CELLP)nil;
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
  //旧世代領域に居るやつは無視する
  if(ISOLDCELLP(cp) || ISOLDATOMP(cp) || ISOLDNUMP(cp)) {
    return cp;
  }
  char c = cp->id;
  //コピー先を表すポインター
  if(!(ISCELLP(cp) || ISATOMP(cp) || ISNUMP(cp)))	 {
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
      if(!(ISCELLP(cp) || ISATOMP(cp) || ISNUMP(cp)))
	{
	  fprintf(stderr, "![%p]car[%p]:", cp, cp->car);
	  //	Copying(&(cp->car), n, a);
	  fprintf(stderr, "![%p]cdr[%p]:", cp, cp->cdr);
	  //	Copying(&(cp->cdr), n, a);
	}
      switch(c) {
      case _ATOM:
	if(a) {
	     if(cp->age >= syoushinn) {
		  cp = promote((CELLP)cp);
	     } else {
		  cp->forwarding = memcpy(freeatomtop, cp, sizeof(ATOM));
		  freeatomtop++;
		  cp->cpflag |= COPIED;
		  ((ATOMP)cp->forwarding)->value = Copy(((ATOMP)cp->forwarding)->value, n, a);
		  ((ATOMP)cp->forwarding)->plist = Copy(((ATOMP)cp->forwarding)->plist, n, a); 
		  if(!((((ATOMP)cp->forwarding)->ftype) & NONMRK)) {
		       ((ATOMP)cp->forwarding)->fptr   = Copy(((ATOMP)cp->forwarding)->fptr, n, a);
		  }
	     }
	     //コピーした際はコピー先のアドレスを返す
	     return cp->forwarding;
	}
	return cp;
      case _CELL:
	   if(cp->age >= syoushinn) {
		cp = promote((CELLP)cp);
	   } else {
		cp->forwarding = memcpy(freecelltop, cp, sizeof(CELL));
		freecelltop++;
		cp->cpflag |= COPIED;
		cp->forwarding->car = Copy(cp->forwarding->car, n, a);
		cp->forwarding->cdr = Copy(cp->forwarding->cdr, n, a);
	   }
	   return cp->forwarding;
      case _FIX:
	   //FALL THROUT
      case _FLT:
	   if(n) {
		if(cp->age >= syoushinn) {
		     cp = promote(cp);
		} else {
		     cp->forwarding = memcpy(freenumtop, cp, sizeof(NUM));
		     freenumtop++;
		     cp->cpflag |= COPIED;
		}
		return cp->forwarding;
	   }
	   return cp;
      defalut:
	return (int)error(ULO);
      }
    }
  }
}


CELLP promote(CELLP cp) {
     //すでに旧世代領域に存在したら何もしない
     if(ISOLDCELLP(cp) || ISOLDATOMP(cp) || ISOLDNUMP(cp)) {
	  return cp;
     }
     //CELLでもATOMでもNUMでも無い物はノータッチ
     if (!(ISCELLP(cp) || ISATOMP(cp) || ISNUMP(cp))) {
	  return cp;
     }
     //それ以外は旧世代領域にコピーしそのポインタを返す
     char c = cp->id;
     switch(c) {
     case _ATOM:
	  if((old_freeatomtop + 1) > old_freeatom + ATOMSIZ) {
	       //もし旧世代領域がいっぱいだったら
	       old_gbc(OFF,ON);
	       if((old_freeatomtop + 1) > old_freeatom + ATOMSIZ) {
		    fprintf(stdout,"Allocation failed!!!!!!\n");
		    exit(0);
	       }
	  }
	  cp->forwarding = memcpy(old_freeatomtop, cp, sizeof(ATOM));
	  old_freeatomtop++;
	  break;
     case _CELL:
	  if((old_freecelltop + 1) > old_freecell + CELLSIZ) {
	       old_gbc(OFF,ON);
	       if((old_freecelltop + 1) > old_freecell + CELLSIZ) {
		    fprintf(stdout,"Allocation failed!!!!!!\n");
		    exit(0);
	       }
	  }
	  cp->forwarding = memcpy(old_freecelltop, cp, sizeof(CELL));
	  old_freecelltop++;
	  break;
     case _FIX:
	  //FALL THROUT
     case _FLT:
	  if((old_freenumtop + 1) > old_freenum + CELLSIZ) {
	       old_gbc(ON,OFF);
	       if((old_freenumtop + 1) > old_freenum + CELLSIZ) {
		    fprintf(stdout,"Allocation failed!!!!!!\n");
		    exit(0);
	       }
	  }
	  cp->forwarding = memcpy(old_freenumtop, cp, sizeof(NUM));
	  old_freenumtop++;
	  break;
     }
     return cp;
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
	  mark(oblist[i], n);
     }
     for(sp1 = stacktop; sp1 <= sp; ++sp1) {
	  mark(*sp1, n);
     }
     i = col_cell(); ec;
     if(n) {
	  n = col_num(); ec;
     }
     if(a) {
	  s = col_str();
	  a = col_atom(); ec;
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
	if(!(ISOLDCELLP(cp) || ISOLDATOMP(cp) || ISOLDNUMP(cp))) {
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
		default:
			return (int)error(ULO);
	}
}

void rem_mark_num() {
	NUMP np;
	for(np = old_freenumtop; np < old_freenumtop + NUMSIZ; ++np) {
		np->id &= FREE;
	}
}

void rem_mark_atom() {
	ATOMP ap;
	for(ap = old_freeatomtop; ap < old_freeatomtop + ATOMSIZ; ++ap) {
		ap->id = _ATOM;
	}
}

int col_cell() 
{
	int n = 1;
	CELLP end, cp = old_freecelltop;
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

int col_num() {
	int n = 1;
	NUMP end, np = old_freenumtop;
	forever {
		if(!(np->id & USED)) {
			break;
		}
		np->id &=FREE;
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

int col_atom() {
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

int col_str()
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

     
