#include <signal.h>
#define MAIN
#include "lisp.h"
#define forever for(;;)

static reset_err();
static init();
static mk_nil();
static mk_sys_atoms();
static greeting();
void refreshCellArea(CELLP, CELLP);
void refreshAtomArea(ATOMP, ATOMP);
void refreshNumArea(NUMP, NUMP);
void main()
{
     cur_fpo = stdout;
     greeting();
     //初期化
     init();
     forever {
	  sp = stacktop;
	  //メインループ
	  if(err == ERR) {
	       pri_err((CELLP)nil);
	  }
	  reset_err();
	  toplevel_f();
	  sp = stacktop;
	  reset_stdin();
     }
}

toplevel_f()
{
     int i;
     CELLP *argp1, *argp2;
     CELLP read_s(), eval(), error();
     //argp1はstacktop+1になる。
     argp1 = ++sp;
     stackcheck;
     //argp2はstacktop+1になる
     argp2 = ++sp;
     forever {
	  //S式を読み込み
	  *argp1 = read_s(TOP);
	  if(*argp1 == (CELLP)eofread) break;
	  ec;
	  //評価して
	  *argp2 = (CELLP)nil;
	  *argp2 = eval(*argp1, *argp2);
	  //エラーが有れば出力し、復帰する
	  switch(err) {
	  case ERROK:
	  case ERR:
	       return (int)(err = ERROK);
	  }
	  //結果を出力
	  print_s(*argp2, ESCON);
	  ec;
	  if(isatty(fileno(cur_fpi))) {
	       fputc('\n',cur_fpo);
	  }
	  /* for(i = 0; i <TABLESIZ; i++) */
	  /*   { */
	  /* 	printf("%2d(%p): ", i, oblist[i]);  */
	  /* 	print_s(oblist[i], ESCON); */
	  /* 	printf("\t"); */
	  /*   } */
	  fputc('\n', cur_fpo);
     }
     //sp(stack pointer)を元の位置に戻す
     sp -= 2;
}

static reset_err()
{
     cur_fpi = stdin;
     cur_fpo = stdout;
     err = NONERR;
     txtp = oneline;
	
     *txtp = '\0';
     for(sp = stacktop; sp < stacktop + STACKSIZ; ++sp) {
	  *sp = (CELLP)nil;
     }
     sp = stacktop - 1;
     verbos = ON;
}

static init() 
{
     char *malloc();
     //	int quit();
     void quit();
     int i;
     CELLP cp;
     ATOMP ap;
     NUMP np;
	
     /*************************************
      *
      *   新世代領域
      *
      *************************************/
     //cell/2の連続領域を確保
     fromcelltop = (CELLP)malloc(sizeof(CELL) * (CELLSIZ / 2));
     tocelltop = (CELLP)malloc(sizeof(CELL) * (CELLSIZ / 2));
     freecelltop = fromcelltop;	
     ///以下同様
     fromatomtop = (ATOMP)malloc(sizeof(ATOM) * (ATOMSIZ / 2));
     toatomtop = (ATOMP)malloc(sizeof(ATOM) * (ATOMSIZ / 2));
     freeatomtop = fromatomtop;
	
     fromnumtop  = (NUMP)malloc(sizeof(NUM) * (NUMSIZ / 2));
     tonumtop = (NUMP)malloc(sizeof(NUM) * (NUMSIZ / 2));
     freenumtop = fromnumtop;
	
     fromstrtop = (STR)malloc(STRSIZ / 2);
     tostrtop = (STR)malloc(STRSIZ / 2);
     freestrtop = fromstrtop;
	
     /*************************************
      *
      *   旧世代領域
      *
      *************************************/
     //cell/2の連続領域を確保
     old_freecell = (CELLP)malloc(sizeof(CELL) * (CELLSIZ));
     old_freecelltop = old_freecell;	
     ///以下同様
     old_freeatom = (ATOMP)malloc(sizeof(ATOM) * (ATOMSIZ));
     old_freeatomtop = old_freeatom;
	
     old_freenum  = (NUMP)malloc(sizeof(NUM) * (NUMSIZ));
     old_freenumtop = old_freenum;
	
     old_freestr = (STR)malloc(STRSIZ);
     old_freestrtop = old_freestr;

     //stackを確保
     sp = stacktop = (CELLP *)malloc(sizeof(CELLP) * STACKSIZ);
	
     //nilを確保する
     nil = (ATOMP)malloc(sizeof(ATOM));
     /*------------------------------領域確保完了------------------------*/

     //領域が確保できているかを確認
     if(fromcelltop == NULL || tocelltop == NULL 
	|| fromatomtop == NULL || toatomtop == NULL
	|| fromnumtop == NULL || tonumtop == NULL 
	|| freestrtop == NULL || tostrtop == NULL 
	|| old_freecell == NULL || old_freeatom == NULL
	|| old_freenum == NULL || old_freestr == NULL
	|| nil == NULL) {
	  printf("Oops! Alloc Error : Too Large Data Area.\n");
	  printf("Please change --SIZ (defined in lisp.h).\n");
	  exit(1);
     }
     refreshCellArea(fromcelltop, tocelltop);
     refreshAtomArea(fromatomtop, toatomtop);
     refreshNumArea(fromnumtop, tonumtop);
     //oblistを初期化
     for(i = 0; i < TABLESIZ; ++i) {
	  oblist[i] = (CELLP)nil;
     }
     mk_sys_atoms();
     ini_subr();
     signal(SIGINT, quit);
}

void refreshCellArea(CELLP from, CELLP to)
{
     CELLP cp;
     //Cellの連結リストを作成
     for(cp = fromcelltop; cp < fromcelltop + (CELLSIZ / 2); ++cp) {
	  cp->id = _CELL;
	  cp->car = (CELLP)nil;
	  cp->cdr = (CELLP)nil;
     }
     //最後尾をnilにする
     (--cp)->cdr = (CELLP)nil;
     for(cp = tocelltop; cp < tocelltop + (CELLSIZ / 2); ++cp) {
	  cp->id = _CELL;
	  cp->car = (CELLP)nil;
	  cp->cdr = (CELLP)nil;
     }
     (--cp)->cdr = (CELLP)nil;
     for(cp = old_freecelltop; cp < old_freecelltop + CELLSIZ; ++cp) {
	  cp->id = _CELL;
	  cp->car = (CELLP)nil;
	  cp->cdr = (CELLP)nil;
     }
     //最後尾をnilにする
     (--cp)->cdr = (CELLP)nil;
}

void refreshAtomArea(ATOMP from, ATOMP to)
{
     ATOMP ap;
     //atomの連結リストを作成
     //nilはcar,cdrともに自分自身を指し示すのでatomの先頭にあるnilは無視する(よって+1から始める)
     for(ap = fromatomtop; ap < fromatomtop + (ATOMSIZ / 2); ++ap) {
	  ap->id = _ATOM;
	  ap->plist = (CELLP)nil;
     }
     //最後尾をnilにする
     (--ap)->plist = (CELLP)nil;
     for(ap = toatomtop; ap < toatomtop + (ATOMSIZ / 2); ++ap) {
	  ap->id = _ATOM;
	  ap->plist = (CELLP)nil;
     }
     //最後尾をnilにする
     (--ap)->plist = (CELLP)nil;
     //旧世代領域も初期か
     for(ap = old_freeatomtop; ap < old_freeatomtop + ATOMSIZ; ++ap) {
	  ap->id = _ATOM;
	  ap->plist = (CELLP)nil;
     }
     (--ap)->plist = (CELLP)nil;
}

void refreshNumArea(NUMP from, NUMP to)
{
     NUMP np;
     //numの連結リストを作成
     for(np = fromnumtop; np < fromnumtop + (NUMSIZ / 2); ++np) {
	  np->id = _FIX;
	  np->value.ptr = (NUMP)nil;
     }
     //最後尾をnilにする
     (--np)->value.ptr = (NUMP)nil;
     for(np = tonumtop; np < tonumtop + (NUMSIZ / 2); ++np) {
	  np->id = _FIX;
	  np->value.ptr = (NUMP)nil;
     }	
     //最後尾をnilにする
     (--np)->value.ptr = (NUMP)nil;
     for(np = old_freenumtop; np < old_freenumtop + NUMSIZ; ++np) {
	  np->id = _FIX;
	  np->value.ptr = (CELLP)nil;
     }
     (--np)->value.ptr = (CELLP)nil;
}	
	
static mk_nil()
{
     char *strcpy();
     char *s = strcpy(freestrtop, "nil");
	
     //末尾の終端文字の分を飛ばす(strlenはNULLは含まない計算をする)
     freestrtop += strlen(s) + 1;
     nil->id = _ATOM;
     nil->value = (CELLP)nil;
     nil->plist = (CELLP)nil;
     nil->name = s;
     nil->ftype = _NFUNC;
     nil->fptr = (CELLP)nil;
     intern(nil);
}

static mk_sys_atoms()
{
     ATOMP mk_atom();
     mk_nil();
     t = mk_atom("t");
     lambda = mk_atom("lambda");
     eofread = mk_atom("EOF");
     prompt = mk_atom("% ");
}

static greeting()
{
     fprintf(stdout,"\n");
     fprintf(stdout,"\t My Lisp Intepreter\n");
     fprintf(stdout,"\t    Developed by Pocket\n");
}

void quit() 
{
     fprintf(stdout, "\n\n\tMay the force be with you!\n");
     fprintf(stdout, "\t\t\tHappy Hacking!!\n");
     exit(0);
}

reset_stdin() 
{
     if(isatty(fileno(stdin))) {
	  rewind(stdin);
     }
     else {
	  freopen("CON", "r", stdin);
     }
}
