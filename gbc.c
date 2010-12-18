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
  //�\���̂��߂̗̈�m�ۂ̏ꍇ
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
  //�V����ATOMP��p�ӂ���Ɣ�������͂ݏo��ꍇ�ɂ�GC�N��
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
  //�V����NUM��ǉ�����ƕ�����͂ݏo��ꍇ�ɂ�GC���N������B
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
  //Cell�͕W����GC�̑Ώۂł���
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

  //fromcelltop��tocelltop����������(Default��GC�̑ΏۂȂ̂�)
  Celltmp = fromcelltop;
  fromcelltop = tocelltop;
  tocelltop = Celltmp;
  //TODO
  //From�̈�̏�����
  if(n) {
    //fromnumtop��tonumtop����������
    Numtmp = fromnumtop;
    fromnumtop = tonumtop;
    tonumtop = Numtmp;
  }
  if(a) {
    //fromatomtop��tocelltop����������
    Atomtmp = fromatomtop;
    fromatomtop = toatomtop;
    toatomtop = Atomtmp;
  }

  // ������̈�́ACOPIED�t���O�����낷�iCopy() �`���̃R�����g�Q�Ɓj
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
/* ���̂Ƃ���number�ɑ΂��Ă͕s�v
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
  //atom�̘A�����X�g���쐬
  //nil��car,cdr�Ƃ��Ɏ������g���w�������̂�atom�̐擪�ɂ���nil�͖�������(�����+1����n�߂�)
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
  //num�̘A�����X�g���쐬
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
  //n = ON�̎���num��Copy����,
  //a = ON�̎���atom,str��Copy����
  *top = Copy(*top, n, a);
}

CELLP Copy(CELLP cp, int n, int a)
{
  //������̈�ɋ����g���́h�͖������邯�ǁA��������w����Ă����͖��������Ⴞ��
  // ��x��������������̈�ɂ����ɂ�COPIED�t���O�����Ăđ��d�ɏ������Ă��܂����Ƃ�h��
  // ��	�A�g��f��fptr�� (lambda (x) (... (f (plus x -1)) ...)) ���͂����Ă��āAf��������
  //	�̈�ɂ���Ƃ��Afptr��Copying�łӂ�����f��Copy���邱�ƂɂȂ�
  // gbc() �̍Ō�ɁA������̈�̂��ׂĂ�cpflag����COPIED�t���O�����낷�����������Ă���
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
printf("������̈�Z��(%p)", cp);
	if(! (cp->cpflag & COPIED))
	{
printf("����̐[��GC");
		cp->cpflag |= COPIED;
		Copying(&(cp->car), n, a);
		Copying(&(cp->cdr), n, a);
	}
printf("\n");
	return cp;
  }
  else if(ISOLDNUMP((NUMP)cp)) // number����w�������̂͂��܂̂Ƃ���Ȃ�
  {
    return cp;
  }
  //char c = cp->id;//N//
  //�R�s�[���\���|�C���^�[
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
			      to = (ATOMP)promote((CELLP)cp);//promote�̓R�s�[��̃A�h���X��Ԃ�
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
		      //�R�s�[�����ۂ̓R�s�[��̃A�h���X��Ԃ�
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
		to = promote((CELLP)cp);//promote�̓R�s�[��̃A�h���X��Ԃ�
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
		     to = (NUMP)promote(cp);//promote�̓R�s�[��̃A�h���X��Ԃ�
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

// �R�s�[��̃A�h���X��Ԃ�����
// �R�s�[�ȂǕK�v�ȏ����͌Ăяo�����iCopy() ���ōs���j
CELLP promote(CELLP cp) {
     char c = cp->id;//N//
     //���łɋ�����̈�ɑ��݂����牽�����Ȃ�
     if(ISOLDCELLP(cp) || ISOLDATOMP((ATOMP)cp) || ISOLDNUMP((NUMP)cp)) {//N//
	  return cp;
     }
     //CELL�ł�ATOM�ł�NUM�ł��������̓m�[�^�b�`
     if (!(ISCELLP(cp) || ISATOMP((ATOMP)cp) || ISNUMP((NUMP)cp))) {//N//
	  return cp;
     }

     //����ȊO�͋�����̈�ւ̃R�s�[��̃A�h���X��Ԃ�
  {  CELLP to;
     switch(c) {
     case _ATOM:
	  if(old_freeatomtop == (ATOMP)nil) {
	       //����������̈悪�����ς���������
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
     return to;//�R�s�[��̃A�h���X��Ԃ�
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
	//������̈�łȂ������ΏۊO
	if(!(ISOLDCELLP(cp) || ISOLDATOMP((ATOMP)cp) || ISOLDNUMP((NUMP)cp))) {//N//
	     return;
	}
	//nil���m�[�^�b�`
	if(cp == (CELLP)nil) {
		return;
	}
	//���ɏ����ς݂̕����ΏۊO
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

     
