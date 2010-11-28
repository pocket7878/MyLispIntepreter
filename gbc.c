#include "lisp.h"
#define forever for(;;)
static void initCellArea(CELLP from);
static void initAtomArea(ATOMP from);
static void initNumArea(NUMP from);
static void Copying(CELLP *top, int n, int a);
static CELLP Copy(CELLP cp, int n, int a);
void gbc(int n, int a);
int on(CELLP* p);
void off(int i);
void gc_aux(int n, int a);

CELLP* cellpp[999];
int cellpptop = 0;

// 登録
int on(CELLP* p)
{
  cellpp [cellpptop] = p;
  return cellpptop ++;
}
// 登録解除
void off(int i)
{
  cellpptop = i;
}

void gc_aux(int n, int a)
{
  int i;
  for(i = 0; i < cellpptop; i++)
    {
      Copying(cellpp[i], n, a);
    }
}

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
	
	
  for(i = 0; i < TABLESIZ; ++i) {
    Copying(&oblist[i], n, a);
  }
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
  printf("** Init Atom Area Start **\n");
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
  printf("** Init Num Area Start **\n");
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
    cp->forwarding = memcpy(old_freeatomtop, cp, sizeof(ATOM));
    old_freeatomtop++;
    break;
  case _CELL:
    cp->forwarding = memcpy(old_freecelltop, cp, sizeof(CELL));
    old_freecelltop++;
    break;
  case _FIX:
    //FALL THROUT
  case _FLT:
    cp->forwarding = memcpy(old_freenumtop, cp, sizeof(NUM));
    old_freenumtop++;
    break;
  }
  return cp->forwarding;
}
