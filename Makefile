CC = gcc
#CC = bcc32
RM = rm
#RM = del
CFLAGS = -g -m32
#CFLAGS = 
BINNAME = repl
#BINNAME = repl.exe
CFILES = calc.c error.c fun.c inisubr.c main.c read.c control.c eval.c gbc.c iofunc.c print.c save.c
#OBJS = calc.obj error.obj fun.obj inisubr.obj main.obj read.obj control.obj eval.obj gbc.obj iofunc.obj print.obj save.obj
#$(BINNAME): $(CFILES)
#$(BINNAME): $(OBJS)
$(BINNAME) : $(CFILES)
	$(CC) $(CFLAGS) -o $(BINNAME) $(CFILES)
#	$(CC) $(CFLAGS) -e$(BINNAME) $(OBJS)

clean: 
	$(RM) $(BINNAME) $(OBJS)
