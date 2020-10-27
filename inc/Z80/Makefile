CC      = gcc
CFLAGS  = -O2 -DLSB_FIRST -DDEBUG

dasm:	dasm.c
	gcc -o dasm dasm.c

Z80.o:	Z80.c Z80.h Codes.h CodesED.h CodesCB.h CodesXX.h Tables.h CodesXCB.h

Debug.o:Debug.c Z80.h
