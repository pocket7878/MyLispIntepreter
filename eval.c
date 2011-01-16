#include <stdlib.h>
#include "lisp.h"
#include "error.h"//N//
#include "gbc.h"//N//
#include "print.h"//N//
#include "save.h"
static int eval_arg_p(ATOMP func);
CELLP apply(CELLP func, CELLP args, CELLP env);
CELLP evallist(CELLP args, CELLP env);
static CELLP push(CELLP keys, CELLP value, CELLP env);
static CELLP atomvalue(ATOMP ap, CELLP env);

CELLP eval(CELLP form, CELLP env)
{
//static int e = 0;
//static char tabs[100];
     CELLP cp, apply(), atomvalue(), evallist();
     ATOMP func;
//tabs[e] = ' ';
//tabs[++e] = '\0';
//printf("\n%s%d: form=", tabs, e);
//printf("省略");
//print_s(form, ESCON);
//printf("\n");
//if(e>150){
//printf(", env=");
//print_s(env, ESCON);
//printf("\n");
//}
     switch(form->id) {
     case _ATOM:
	  cp = atomvalue((ATOMP)form, env);
	  break;
     case _FIX:
     case _FLT:
//printf("\n%s%d: result=(NUM)", tabs, e);
//print_s(form,ESCON);
//printf("\n");
//tabs[--e] = '\0';
	  return form;
     case _CELL:
//	  stackcheck;
	  //スタックポインタを進める
	  *++sp = (CELLP)nil;	stackcheck;
//printf("=%d= ", __LINE__);
	  func = (ATOMP)form->car;
//printf("=%d= ", __LINE__);
	  {//N//
	    int q = on(&form);
	    on(&env);
	    //on(sp);//N//
	    on((CELLP*)&func);//N//
//printf("=%d= ", __LINE__);
	    if(eval_arg_p(func)) {
	         //スタックに引き数を評価した結果を保存する(このバックグラウンドでspは--されている)
//printf("eval form=");
//print_s(form, ESCOFF);
//printf("=%d= ", __LINE__);
	         *sp = evallist(form->cdr, env);
//printf("=%d= ", __LINE__);
	         //off(q);//N//
	         if(err){//N//
	           off(q);//N//
//printf("=%d= ", __LINE__);
	           break;//N//
	         }//N//
	    }
	    else {
//printf("=%d= ", __LINE__);
	         *sp = form->cdr;
	    }
//	    printf("\nEVAL: Current *SP is ");
//	    print_s(*sp, ESCON);
//	    printf("\n");
//printf("=%d= ", __LINE__);
	    cp = apply((CELLP)func, *sp, env);
//printf("=%d= ", __LINE__);
	    off(q);
	  }//N//
	  sp--;
	  break;
     default:
//printf("\n%s%d: result=EORROR", tabs, e);
//tabs[--e] = '\0';
	  error(ULO);

     }
     if(err == ERR) {
	  pri_err(form);
//printf("\n%s%d: result=EORROR", tabs, e);
//tabs[--e] = '\0';
	  return NULL;
     }
//printf("\n%s%d: result=", tabs, e);
//printf("省略");
//print_s(cp, ESCON);
//tabs[--e] = '\0';
//if(e == 0) printf("\n");
//printf("=%d= ", __LINE__);
     return cp;
}

static int eval_arg_p(ATOMP func) 
{
     if(func->id == _ATOM && func->ftype & _EA) {
	  return TRUE;
     }
     if(func->id == _CELL && ((CELLP)func)->car == (CELLP)lambda) {
	  return TRUE;
     }
     return FALSE;
}

//function, args, environment!!
CELLP apply(CELLP func, CELLP args, CELLP env)
{
     //printf("\nAPPLY: Current args: \n");//N//
     //print_s(args, ESCON);//N//
     //printf("\n");//N//
     CELLP (*funcp)(), bodies, result = (CELLP)nil;
     //CELLP bind();//N//
     CELLP bind(CELLP keys, CELLP values, CELLP env);//N//
     char funtype;

//printf("=%d= ", __LINE__);
//printf("apply func=");
//print_s(func, ESCOFF);
//printf(" args=");
//print_s(args, ESCOFF);
//printf(" env=");
//print_s(env, ESCOFF);
   //  printf("\nAPPLY: Current args: \n");//N//
    // print_s(args, ESCON);//N//
     //printf("\n");//N//

     //function check
     switch(func->id) {
     case _ATOM:
//printf("=%d= ", __LINE__);
	  //if func is atom => maybe just func
	  funtype = ((ATOMP)func)->ftype;
	  if(funtype & _UD) {
//printf("=%d= ", __LINE__);
	       return error(UDF);
	  }
	  if(funtype & _SR) {
//printf("=%d= ", __LINE__);
	       funcp = (CELLP (*)())((ATOMP)func)->fptr;
	       if(funtype & _EA) {
//printf("=%d= ", __LINE__);
		    return (*(CELLP(*)(CELLP))funcp)(args);//N//
	       }
	       else {
//printf("=%d= ", __LINE__);
		    return (*(CELLP(*)(CELLP,CELLP))funcp)(args, env);//N//
	       }
	  }
	  func = ((ATOMP)func)->fptr;
//printf("=%d= ", __LINE__);
     case _CELL:
//printf("=%d= ", __LINE__);
//printf("func=");
//print_s(func, ESCOFF);
	  //if func is cell => maybe lambda
	  //(lambda (x) <- this must be cell
	  if(func->cdr->id != _CELL) {
//printf("=%d= ", __LINE__);
	       return error(IFF);
	  }
	  //(lambda <- check!!
	  if(func->car == (CELLP)lambda) {
	       int q;//N//
	       //body (lambda (x) (hoge hoge) <- body!!
	       bodies = func->cdr->cdr;
	       stackcheck;
				
	       //lambda-argsの引き数のそれぞれにargsの値をbindするよ :-)!!
	       q = on(&args);//N//
	       on(&env);
	       on(&func);
	       on(&bodies);
	       *++sp = bind(func->cdr->car, args, env);//N//
	       off(q);//N//
		       ec;//N//
	       for(; bodies->id == _CELL; bodies = bodies->cdr) {
		    q = on(&args);
		    on(&env);
		    on(&func);
		    on(&bodies);
		    result = eval(bodies->car, *sp);//N//
		    off(q);//N//
			    ec;//N//
	       }
	       sp--;
//printf("=%d= ", __LINE__);
	       return result;
	  }
     default:
//printf("=%d= ", __LINE__);
	  return error(IFF);
     }
}

CELLP evallist(CELLP args, CELLP env)
{
     int q;
     CELLP cp1, newcell(), eval();
     //引き数のリストがcellでない場合はおそらくnilなのでnilを返却する
//printf("evallist args=");
//print_s(args, ESCOFF);
     if(args->id != _CELL) {
	  return (CELLP)nil;
     }
     stackcheck;
     q = on(&args);
     on(&env);
     //stackに新しいcellを用意する
//printf("=%d= ", __LINE__);
     *++sp = newcell();//N//
//printf("=%d= ", __LINE__);
     off(q);//N//
	     ec;//N//
     //現在のスタックポインタを一旦保存しておく
     cp1 = *sp;
     //保存したcellのcarに引き数の一つ目を評価した物を入れる
     q = on(&cp1); //CP1を追加保護
     on(&args);
     on(&env);
//printf("=%d= ", __LINE__);
     cp1->car = eval(args->car, env);//N//
//printf("=%d= ", __LINE__);
     off(q);//N//
	     ec;//N//
     //次の引き数に移る
//printf("\n***(*sp)は%p番地に存在し、内容は%pである。そこには", sp, *sp);print_s(*sp,ESCOFF);printf("がある。");

     args = args->cdr;
     //引き数がcell型である限り、処理を進める
//printf("=%d= ", __LINE__);
     while(args->id == _CELL) {
//printf("=%d= ", __LINE__);
	  q = on(&env);
	  on(&args);
	  on(&cp1);
	  //保存したcellのcdrに新しいcellを確保する
//printf("\n***2つ目以降の引数のためにnewcell()を呼び出しcp1（*sp1と同じ）のcdrにつなぐ。");
//printf("=%d= ", __LINE__);
	  cp1->cdr = newcell();//N//
//printf("=%d= ", __LINE__);
//print_s(args, ESCOFF);
//printf("\n***(*sp)は%p番地に存在し、内容は%pである。そこには", sp, *sp);print_s(*sp,ESCOFF);printf("がある。");

//printf("\n***cp1は%p番地に存在し、内容は%pである。そこには", &cp1, cp1);print_s(cp1,ESCOFF);printf("がある。");

	  off(q);//N//
//printf("=%d= ", __LINE__);
	     ec;//N//
//printf("=%d= ", __LINE__);
	  //保存したcellのcdrに評価結果を入れる
//printf("=%d= ", __LINE__);
	  cp1 = cp1->cdr;
//printf("=%d= ", __LINE__);
	  q = on(&env);
	  on(&args);
	  on(&cp1);
//printf("\n***cp1は%p番地に存在し、内容は%pである。そこには", &cp1, cp1);print_s(cp1,ESCOFF);printf("がある。");

//printf("\n***argsは%p番地に存在し、内容は%pである。そこには", &args, args);print_s(args,ESCOFF);printf("がある。");
//printf("\n***envは%p番地に存在し、内容は%pである。そこには", &env, env);print_s(env,ESCOFF);printf("がある。");
//printf("\n***args->carをenvのもとでevalしたところ、その結果tmpは…");
//printf("=%d= ", __LINE__);
	  {
	  CELLP tmp = eval(args->car, env);
//printf("=%d= ", __LINE__);
//printf("\n***evalした結果tmpは%p番地に存在し、内容は%pである。そこには", &tmp, tmp);print_s(tmp,ESCOFF);printf("がある。");
//printf("\n***evalを経て、cp1、args、envは…");
//printf("\n***cp1は%p番地に存在し、内容は%pである。そこには", &cp1, cp1);print_s(cp1,ESCOFF);printf("がある。");
//printf("\n***argsは%p番地に存在し、内容は%pである。そこには", &args, args);print_s(args,ESCOFF);printf("がある。");
//printf("\n***envは%p番地に存在し、内容は%pである。そこには", &env, env);print_s(env,ESCOFF);printf("がある。");
	  cp1->car = tmp;
//printf("\n***cp1->carにtmpを代入して…");
//printf("\n***cp1は%p番地に存在し、内容は%pである。そこには", &cp1, cp1);print_s(cp1,ESCOFF);printf("がある。");
//printf("\n***(*sp)は%p番地に存在し、内容は%pである。そこには", sp, *sp);print_s(*sp,ESCOFF);printf("がある。");

	  }//N//
	  off(q);//N//
//printf("=%d= ", __LINE__);
	     ec;//N//
//printf("=%d= ", __LINE__);
	  args = args->cdr;
//printf("=%d= ", __LINE__);
     }
     //これを抜けた辞典でスタックにはすべての引き数の評価結果が入っているそしてnilでしめる。
//printf("=%d= ", __LINE__);
     cp1->cdr = (CELLP)nil;
     //スタックポインタを返し、その後spを一つ減らす
//printf("=%d= ", __LINE__);
     return *sp--;
}

//keysのそれぞれにvaluesのそれぞれの値をbindするよ！！
CELLP bind(CELLP keys, CELLP values, CELLP env)
{
     CELLP push();
     int q;//N//
 //    printf("\nBIND: Current keys");
     //print_s(keys,ESCON);
     //printf("\n");
  //   printf("\nBIND: Current values");
     //print_s(values,ESCON);
     //printf("\n");
     //q = on(&env);//N//
     //on(&keys);//N//
     //on(&values);//N//
     //keysががnilでなくかつ、keysがatomなら
     if(keys != (CELLP)nil && keys->id == _ATOM) {
	  q = on(&env);//N//
	  on(&keys);//N//
	  on(&values);//N//
	  env = push(keys, values, env);//N//
	  off(q);//N//
		  ec;//N//
	  return env;
     }
     //off(q);//N//
     stackcheck;
     *++sp = env;
     while(keys->id == _CELL) {
	  if(values->id != _CELL) {
	       return error(NEA);
	  }
	  q = on(&env);
	  on(&keys);
	  on(&values);
	  *sp = push(keys->car, values->car, *sp);//N//
	  off(q);//N//
		  ec;//N//
	  keys = keys->cdr;
	  values = values->cdr;
     }
     if(keys != (CELLP)nil && keys->id == _ATOM) {
	  q = on(&env);
	  on(&keys);
	  on(&values);
	  *sp = push(keys, values, *sp);//N//
	  off(q);//N//
		  ec;//N//
     }
     return *sp--;
}

static CELLP push(CELLP keys, CELLP value, CELLP env)
{
     int q;
     CELLP newcell();
//     printf("\nPUSH: Current Value is: ");
     //print_s(value, ESCOFF);
     //printf("\n");
//     printf("\nPUSH: Current *SP is: ");
     //print_s(*sp, ESCOFF);
     //printf("\n");
     stackcheck;
     q = on(&env);
     on(&keys);
     on(&value);
//printf("=%d= ", __LINE__);
     *++sp = newcell();//N//
//printf("=%d= ", __LINE__);
     off(q);//N//
	  ec;//N//
     (*sp)->cdr = env;
     env = *sp;
     q = on(&env);
     on(&keys);
     on(&value);
//printf("=%d= ", __LINE__);
     env->car = newcell();//N//
//printf("=%d= ", __LINE__);
     off(q);//N//
	  ec;//N//
     env->car->car = keys;
     env->car->cdr = value;
     return *sp--;
}

static CELLP atomvalue(ATOMP ap, CELLP env)
{
     while(env->id == _CELL) {
	  if(env->car->id != _CELL) {
	       return error(EHA);
	  }
	  if(env->car->car == (CELLP)ap) {
	       return env->car->cdr;
	  }
	  env = env->cdr;
     }
     return ap->value;
}

