#CC = gcc
CC = bcc32
#RM = rm
RM = del
#CFLAGS = -g 
CFLAGS = -N 
#BINNAME = repl
BINNAME = repl.exe
#CFILES = calc.c error.c fun.c inisubr.c main.c read.c control.c eval.c gbc.c iofunc.c print.c save.c
OBJS = calc.obj error.obj fun.obj inisubr.obj main.obj read.obj control.obj eval.obj gbc.obj iofunc.obj print.obj save.obj prog.obj fun3.obj

$(BINNAME): $(OBJS)
	$(CC) $(CFLAGS) -e$(BINNAME) $(OBJS)
#$(BINNAME) : $(CFILES)
#	$(CC) $(CFLAGS) -o $(BINNAME) $(CFILES)

clean: 
	$(RM) $(BINNAME) $(OBJS)
