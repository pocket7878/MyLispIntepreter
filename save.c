#include "save.h"
#include "gbc.h"

int on(CELLP* p)
{
     cellpp[cellpptop] = p;
     return cellpptop++;
}

void off(int i)
{
     cellpptop = i;
}

void gc_aux(int n, int a)
{
     int i;
     fprintf(stdout,"[GC GC_AUX start.]\n");
     for(i = 0; i < cellpptop; i++) {
	  Copying(cellpp[i], n, a);
     }
     fprintf(stdout,"[GC GC_AUX end.]\n");
}

void old_gc_aux(int n)
{
     int i;
//     fprintf(stdout,"[OLDGC OLD_GC_AUX start.]\n");
     for(i = 0; i < cellpptop; i++) {
	  mark(*cellpp[i], n);
     }
//     fprintf(stdout,"[OLDGC OLD_GC_AUX end.]\n");
}
