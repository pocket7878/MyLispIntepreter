#include <signal.h>
#include <stdlib.h>//N//
#include <stdio.h>//N//
#include <io.h>//N//
#include <string.h>//N//
#define MAIN
#include "lisp.h"
#include "error.h"//N//
#include "read.h"//N//
#include "print.h"//N//
#include "fun.h"//N//
#include "save.h"
#define forever for(;;)
//int save_in_sys_atom = 0;//N//save_in_sys_atomはdefvar.h（つまりmain.c）内で定義される
static void reset_err(void );//N//
static void init(void );//N//
static void mk_nil(void );//N//
static void mk_sys_atoms(void );//N//
static void greeting(void );//N//
void refreshCellArea(CELLP, CELLP);
void refreshAtomArea(ATOMP, ATOMP);
void refreshNumArea(NUMP, NUMP);
int toplevel_f(void);//N//
static void reset_err(void);//N//
static void init(void);//N// 
void reset_stdin(void);//N//
void quit(void);//N//

CELLP eval(CELLP form, CELLP env);//N//

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

#define ISCELLP(x) (fromcelltop <= (x) && (x) < fromcelltop + (CELLSIZ / 2))

int toplevel_f(void)//N//
{
     int i;
     CELLP *argp1, *argp2;
     CELLP read_s(), eval(), error();
//     int q;//N//
     //argp1はstacktop+1になる。
     argp1 = ++sp;
//     q = on(argp1);//N//
     stackcheck;
     //argp2はstacktop+1になる
     argp2 = ++sp;
//     on(argp2);
//     on(sp);
     forever {
	  //S式を読み込み
	  *argp1 = read_s(TOP);
	  if(*argp1 == (CELLP)eofread) break;
	  ec;
	  //評価して
	  *argp2 = (CELLP)nil;
//	  on(argp1);
//	  on(argp2);
//print_s(((ATOMP)((*argp1)->car))->fptr, ESCOFF);
print_s((*argp1)->car, ESCOFF);
printf("[%p]",(ATOMP)((*argp1)->car));
if(ISCELLP(((ATOMP)((*argp1)->car))->fptr)) print_s(((ATOMP)((*argp1)->car))->fptr, ESCOFF);
	  *argp2 = eval(*argp1, *argp2);
//	  off(q);
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
//     off(q);
     //sp(stack pointer)を元の位置に戻す
     sp -= 2;
     return 0;//N//
}

static void reset_err(void)//N//
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

static void init(void)//N// 
{
//     char *malloc();//N//#inclue <stdlib.h>をファイル先頭に加えた
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
     //予約語用の領域を確保する
     freesysatomtop = freesysatom = (ATOMP)malloc(sizeof(ATOM) * (SYSATOMS));
     /*------------------------------領域確保完了------------------------*/

     //領域が確保できているかを確認
     if(fromcelltop == NULL || tocelltop == NULL 
	|| fromatomtop == NULL || toatomtop == NULL
	|| fromnumtop == NULL || tonumtop == NULL 
	|| freestrtop == NULL || tostrtop == NULL 
	|| old_freecell == NULL || old_freeatom == NULL
	|| old_freenum == NULL || old_freestr == NULL
	|| nil == NULL || freesysatomtop == NULL || freesysatom == NULL) {
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
	  cp->forwarding = (CELLP)nil;
	  cp->car = (CELLP)nil;
	  cp->cdr = (CELLP)nil;
     }
     //最後尾をnilにする
     (--cp)->cdr = (CELLP)nil;
     for(cp = tocelltop; cp < tocelltop + (CELLSIZ / 2); ++cp) {
	  cp->id = _CELL;
	  cp->forwarding = (CELLP)nil;
	  cp->car = (CELLP)nil;
	  cp->cdr = (CELLP)nil;
     }
     (--cp)->cdr = (CELLP)nil;
     for(cp = old_freecelltop; cp < old_freecelltop + CELLSIZ; ++cp) {
	  cp->id = _CELL;
	  cp->forwarding = (CELLP)nil;
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
	  ap->age = 0;//N//
	  ap->cpflag = NOTCOPIED;
	  ap->forwarding = (CELLP)nil;//N//
	  ap->value = (CELLP)nil;//N//
	  ap->plist = (CELLP)nil;
	  ap->name = "";//N//
	  ap->ftype = 0;//N//
	  ap->fptr = (CELLP)nil;//N//
     }
     //最後尾をnilにする
     (--ap)->plist = (CELLP)nil;
     for(ap = toatomtop; ap < toatomtop + (ATOMSIZ / 2); ++ap) {
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
     //最後尾をnilにする
     (--ap)->plist = (CELLP)nil;
     //旧世代領域も初期か
     for(ap = old_freeatomtop; ap < old_freeatomtop + ATOMSIZ; ++ap) {
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
     for(ap = freesysatomtop; ap < freesysatomtop + SYSATOMS; ++ap) {
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
}

void refreshNumArea(NUMP from, NUMP to)
{
     NUMP np;
     //numの連結リストを作成
     for(np = fromnumtop; np < fromnumtop + (NUMSIZ / 2); ++np) {
	  np->id = _FIX;
	  np->forwarding = (CELLP)nil;//N//
	  np->value.ptr = (NUMP)nil;
     }
     //最後尾をnilにする
     (--np)->value.ptr = (NUMP)nil;
     for(np = tonumtop; np < tonumtop + (NUMSIZ / 2); ++np) {
	  np->id = _FIX;
	  np->forwarding = (CELLP)nil;//N//
	  np->value.ptr = (NUMP)nil;
     }	
     //最後尾をnilにする
     (--np)->value.ptr = (NUMP)nil;
     for(np = old_freenumtop; np < old_freenumtop + NUMSIZ; ++np) {
	  np->id = _FIX;
	  np->forwarding = (CELLP)nil;//N//
	  np->value.ptr = (NUMP)nil;//N//
     }
     (--np)->value.ptr = (NUMP)nil;//N//
}	
	
static void mk_nil(void)//N//
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

static void mk_sys_atoms(void)
{
     ATOMP mk_atom();
     save_in_sys_atom = 1;
     mk_nil();
     t = mk_atom("t");
     lambda = mk_atom("lambda");
     eofread = mk_atom("EOF");
     prompt = mk_atom("% ");
     save_in_sys_atom = 0;
}

static void greeting(void)//N//
{
     fprintf(stdout,"\n");
     fprintf(stdout,"\t My Lisp Intepreter\n");
     fprintf(stdout,"\t    Developed by Pocket\n");
}

void quit(void)//N//
{
     fprintf(stdout, "\n\n\tMay the force be with you!\n");
     fprintf(stdout, "\t\t\tHappy Hacking!!\n");
     exit(0);
}

void reset_stdin(void)//N//
{
     if(isatty(fileno(stdin))) {
	  rewind(stdin);
     }
     else {
	  freopen("CON", "r", stdin);
     }
}
