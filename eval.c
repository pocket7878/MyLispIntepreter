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
     tabs[e] = ' ';
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
	  //�X�^�b�N�|�C���^��i�߂�
	  *++sp = (CELLP)nil;
	  func = (ATOMP)form->car;
	  {//N//
	    int q = on(&form);
	    on(&env);
	    //on(sp);//N//
	    on((CELLP*)&func);//N//
	    if(eval_arg_p(func)) {
	         //�X�^�b�N�Ɉ�������]���������ʂ�ۑ�����(���̃o�b�N�O���E���h��sp��--����Ă���)
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
				
	       //lambda-args�̈������̂��ꂼ���args�̒l��bind����� :-)!!
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
     //�������̃��X�g��cell�łȂ��ꍇ�͂����炭nil�Ȃ̂�nil��ԋp����
     if(args->id != _CELL) {
	  return (CELLP)nil;
     }
     stackcheck;
     q = on(&args);
     on(&env);
     //stack�ɐV����cell��p�ӂ���
     *++sp = newcell();//N//
     off(q);//N//
	     ec;//N//
     //���݂̃X�^�b�N�|�C���^����U�ۑ����Ă���
     cp1 = *sp;
     //�ۑ�����cell��car�Ɉ������̈�ڂ�]��������������
     q = on(&cp1); //CP1��ǉ��ی�
     on(&args);
     on(&env);
     cp1->car = eval(args->car, env);//N//
     off(q);//N//
	     ec;//N//
     //���̈������Ɉڂ�
printf("\n***(*sp)��%p�Ԓn�ɑ��݂��A���e��%p�ł���B�����ɂ�", sp, *sp);print_s(*sp,ESCOFF);printf("������B");

     args = args->cdr;
     //��������cell�^�ł������A������i�߂�
     while(args->id == _CELL) {
	  q = on(&env);
	  on(&args);
	  on(&cp1);
	  //�ۑ�����cell��cdr�ɐV����cell���m�ۂ���
printf("\n***2�ڈȍ~�̈����̂��߂�newcell()���Ăяo��cp1�i*sp1�Ɠ����j��cdr�ɂȂ��B");
	  cp1->cdr = newcell();//N//
printf("\n***(*sp)��%p�Ԓn�ɑ��݂��A���e��%p�ł���B�����ɂ�", sp, *sp);print_s(*sp,ESCOFF);printf("������B");

printf("\n***cp1��%p�Ԓn�ɑ��݂��A���e��%p�ł���B�����ɂ�", &cp1, cp1);print_s(cp1,ESCOFF);printf("������B");

	  off(q);//N//
	     ec;//N//
	  //�ۑ�����cell��cdr�ɕ]�����ʂ�����
	  cp1 = cp1->cdr;
	  q = on(&env);
	  on(&args);
	  on(&cp1);
printf("\n***cp1��%p�Ԓn�ɑ��݂��A���e��%p�ł���B�����ɂ�", &cp1, cp1);print_s(cp1,ESCOFF);printf("������B");

printf("\n***args��%p�Ԓn�ɑ��݂��A���e��%p�ł���B�����ɂ�", &args, args);print_s(args,ESCOFF);printf("������B");
printf("\n***env��%p�Ԓn�ɑ��݂��A���e��%p�ł���B�����ɂ�", &env, env);print_s(env,ESCOFF);printf("������B");
printf("\n***args->car��env�̂��Ƃ�eval�����Ƃ���A���̌���tmp�́c");
	  {
	  CELLP tmp = eval(args->car, env);
printf("\n***eval��������tmp��%p�Ԓn�ɑ��݂��A���e��%p�ł���B�����ɂ�", &tmp, tmp);print_s(tmp,ESCOFF);printf("������B");
printf("\n***eval���o�āAcp1�Aargs�Aenv�́c");
printf("\n***cp1��%p�Ԓn�ɑ��݂��A���e��%p�ł���B�����ɂ�", &cp1, cp1);print_s(cp1,ESCOFF);printf("������B");
printf("\n***args��%p�Ԓn�ɑ��݂��A���e��%p�ł���B�����ɂ�", &args, args);print_s(args,ESCOFF);printf("������B");
printf("\n***env��%p�Ԓn�ɑ��݂��A���e��%p�ł���B�����ɂ�", &env, env);print_s(env,ESCOFF);printf("������B");
	  cp1->car = tmp;
printf("\n***cp1->car��tmp�������āc");
printf("\n***cp1��%p�Ԓn�ɑ��݂��A���e��%p�ł���B�����ɂ�", &cp1, cp1);print_s(cp1,ESCOFF);printf("������B");
printf("\n***(*sp)��%p�Ԓn�ɑ��݂��A���e��%p�ł���B�����ɂ�", sp, *sp);print_s(*sp,ESCOFF);printf("������B");

	  }//N//
	  off(q);//N//
	     ec;//N//
	  args = args->cdr;
     }
     //����𔲂������T�ŃX�^�b�N�ɂ͂��ׂĂ̈������̕]�����ʂ������Ă��邻����nil�ł��߂�B
     cp1->cdr = (CELLP)nil;
     //�X�^�b�N�|�C���^��Ԃ��A���̌�sp������炷
     return *sp--;
}

//keys�̂��ꂼ���values�̂��ꂼ��̒l��bind�����I�I
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
     //keys����nil�łȂ����Akeys��atom�Ȃ�
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

