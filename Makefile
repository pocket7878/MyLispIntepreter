CC = gcc
#CC = bcc32
CFLAGS = -w -g -m32
#CFLAGS = 
BINNAME = repl
CFILES = calc.c error.c fun.c inisubr.c main.c read.c control.c eval.c gbc.c iofunc.c print.c
repl: $(CFILES)
#	$(CC) $(CFLAGS) -e$(BINNAME).exe $(CFILES)
	$(CC) $(CFLAGS) -o $(BINNAME) $(CFILES)

clean: 
	rm $(BINNAME)
