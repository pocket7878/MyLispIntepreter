int num(STR x);
int isesc(uchar c);
int isprkana(uchar c);
int iskanji(uchar c);
int iskanji2(uchar c);
CELLP read_s(int level);
ATOMP old_atom(STR nam);
ATOMP mk_atom(STR nam);
void intern(ATOMP ap);
