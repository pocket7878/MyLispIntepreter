#include "save.h"
void Copying(CELLP *top, int n, int a);

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
     printf("GC_AUX start**\n");
     for(i = 0; i < cellpptop; i++) {
	  Copying(cellpp[i], n, a);
     }
}
