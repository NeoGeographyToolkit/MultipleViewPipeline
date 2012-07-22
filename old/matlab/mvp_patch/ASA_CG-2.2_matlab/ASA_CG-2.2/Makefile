default: all

#CC = gcc -lm -W -Wall -pedantic -Wmissing-prototypes \
#	-Wredundant-decls -Wnested-externs -Wdisabled-optimization \
#	-ansi -g -fexceptions -Wno-parentheses -Wshadow -Wcast-align \
#	-Winline -Wstrict-prototypes -Wno-unknown-pragmas

# for speed:
CC = gcc -lm -O3

all: driver1

driver1: Makefile driver1.c asa_cg.o
	$(CC) driver1.c -o driver1 asa_cg.o

asa_cg.o: asa_cg.c
	$(CC) -c $*.c

clean:
	rm *.o

purge:
	rm *.o driver1
