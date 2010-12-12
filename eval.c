#include "lisp.h"
#include "error.h"//N//
#include "gbc.h"//N//
#include "print.h"//N//
#include "save.h"
static int eval_arg_p(ATOMP func);
static CELLP apply(CELLP func, CELLP args, CELLP env);
static CELLP evallist(CELLP args, CELLP env);
static CELLP push(CELLP keys, CELLP value, CELLP env);
static CELLP atomvalue(ATOMP ap, CELLP env);

CELLP eval(CELLP form, CELLP env)
{
     static int e = 0;
     static char tabs[100];
     CELLP cp, apply(), atomvalue(), evallist();
     ATOMP func;
     tabs[e] = '\t';
     tabs[++e] = '\0';
     printf("\n%s%d: form=", tabs, e);
     print_s(form, ESCON);
     printf(", env=");
     print_s(env, ESCON);

     switch(form->id) {
     case _ATOM:
	  cp = atomvalue((ATOMP)form, env);
	  break;
     case _FIX:
     case _FLT:
	  printf("\n%s%d: result=(NUM)", tabs, e);
	  print_s(form,ESCON);
	  tabs[--e] = '\0';
	  return form;
     case _CELL:
	  stackcheck;
	  //スタックポインタを進める
	  ++sp;
	  func = (ATOMP)form->car;
	  {//N//
	    int q = on(&form);
	    on(&env);
	    //on(sp);//N//
	    on((CELLP*)&func);//N//
	    if(eval_arg_p(func)) {
	         //スタックに引き数を評価した結果を保存する(このバックグラウンドでspは--されている)
	         *sp = evallist(form->cdr, env);
	         //off(q);//N//
	         if(err){//N//
	           off(q);//N//
	           break;//N//
	         }//N//
	    }
	    else {
	         *sp = form->cdr;
	    }
	    printf("\nEVAL: Current *SP is ");
	    print_s(*sp, ESCON);
	    printf("\n");
	    cp = apply((CELLP)func, *sp, env);
	    off(q);
	  }//N//
	  sp--;
	  break;
     default:
	  printf("\n%s%d: result=EORROR", tabs, e);
	  tabs[--e] = '\0';
	  error(ULO);

     }
     if(err == ERR) {
	  pri_err(form);
	  printf("\n%s%d: result=EORROR", tabs, e);
	  tabs[--e] = '\0';
	  return NULL;
     }
     printf("\n%s%d: result=", tabs, e);
     print_s(cp, ESCON);
     tabs[--e] = '\0';
     if(e == 0) printf("\n");
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
static CELLP apply(CELLP func, CELLP args, CELLP env)
{
     //printf("\nAPPLY: Current args: \n");//N//
     //print_s(args, ESCON);//N//
     //printf("\n");//N//
     CELLP (*funcp)(), bodies, result = (CELLP)nil;
     //CELLP bind();//N//
     CELLP bind(CELLP keys, CELLP values, CELLP env);//N//
     char funtype;

     printf("\nAPPLY: Current args: \n");//N//
     print_s(args, ESCON);//N//
     printf("\n");//N//

     //function check
     switch(func->id) {
     case _ATOM:
	  //if func is atom => maybe just func
	  funtype = ((ATOMP)func)->ftype;
	  if(funtype & _UD) {
	       return error(UDF);
	  }
	  if(funtype & _SR) {
	       funcp = (CELLP (*)())((ATOMP)func)->fptr;
	       if(funtype & _EA) {
		    return (*(CELLP(*)(CELLP))funcp)(args);//N//
	       }
	       else {
		    return (*(CELLP(*)(CELLP,CELLP))funcp)(args, env);//N//
	       }
	  }
	  func = ((ATOMP)func)->fptr;
     case _CELL:
	  //if func is cell => maybe lambda
	  //(lambda (x) <- this must be cell
	  if(func->cdr->id != _CELL) {
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
	       return result;
	  }
     default:
	  return error(IFF);
     }
}

static CELLP evallist(CELLP args, CELLP env)
{
     int q;
     CELLP cp1, newcell(), eval();
     //引き数のリストがcellでない場合はおそらくnilなのでnilを返却する
     if(args->id != _CELL) {
	  return (CELLP)nil;
     }
     stackcheck;
     q = on(&args);
     on(&env);
     //stackに新しいcellを用意する
     *++sp = newcell();//N//
     off(q);//N//
	     ec;//N//
     //現在のスタックポインタを一旦保存しておく
     cp1 = *sp;
     //保存したcellのcarに引き数の一つ目を評価した物を入れる
     q = on(&cp1); //CP1を追加保護
     on(&args);
     on(&env);
     cp1->car = eval(args->car, env);//N//
     off(q);//N//
	     ec;//N//
     //次の引き数に移る
     args = args->cdr;
     //引き数がcell型である限り、処理を進める
     while(args->id == _CELL) {
	  q = on(&env);
	  on(&args);
	  on(&cp1);
	  //保存したcellのcdrに新しいcellを確保する
	  cp1->cdr = newcell();//N//
	  off(q);//N//
	     ec;//N//
	  //保存したcellのcdrに評価結果を入れる
	  cp1 = cp1->cdr;
	  q = on(&env);
	  on(&args);
	  on(&cp1);
	  cp1->car = eval(args->car, env);//N//
	  off(q);//N//
	     ec;//N//
	  args = args->cdr;
     }
     //これを抜けた辞典でスタックにはすべての引き数の評価結果が入っているそしてnilでしめる。
     cp1->cdr = (CELLP)nil;
     //スタックポインタを返し、その後spを一つ減らす
     return *sp--;
}

//keysのそれぞれにvaluesのそれぞれの値をbindするよ！！
CELLP bind(CELLP keys, CELLP values, CELLP env)
{
     CELLP push();
     int q;//N//
     printf("\nBIND: Current keys");
     print_s(keys,ESCON);
     printf("\n");
     printf("\nBIND: Current values");
     print_s(values,ESCON);
     printf("\n");
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
     printf("\nPUSH: Current Value is: ");
     print_s(value, ESCOFF);
     printf("\n");
     printf("\nPUSH: Current *SP is: ");
     print_s(*sp, ESCOFF);
     printf("\n");
     stackcheck;
     q = on(&env);
     on(&keys);
     on(&value);
     *++sp = newcell();//N//
     off(q);//N//
	  ec;//N//
     (*sp)->cdr = env;
     env = *sp;
     q = on(&env);
     on(&keys);
     on(&value);
     env->car = newcell();//N//
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

