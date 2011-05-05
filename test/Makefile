# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# if you are cross-compiling, you need to point to the right executables
# and change the flags to ld and the build procedure for as

#GCCDIR = ../../../gnu-decstation-ultrix/decstation-ultrix/2.95.3/
GCCDIR = ../cs/class/cs270t/gcc/2.95.3/

LDFLAGS = -T script -N
ASFLAGS = -mips2
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
CPPFLAGS = $(INCDIR)

PATH = $(GCCDIR):/lib:/usr/bin:/bin

#CC = $(GCCDIR)gcc -B../../../gnu-decstation-ultrix/
CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld

CPP = $(CC) -E
INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: shell matmult sort halt halt2 cat cp mix_test mix_test1 mix_test2 ncat org\
	test1

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.c > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

shell.o: shell.c
	$(CC) $(CFLAGS) -c shell.c
shell: shell.o start.o
	$(LD) $(LDFLAGS) start.o shell.o -o shell.coff
	../bin/coff2noff shell.coff shell

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

halt2.o: halt2.c
	$(CC) $(CFLAGS) -c halt2.c
halt2: halt2.o start.o
	$(LD) $(LDFLAGS) start.o halt2.o -o halt2.coff
	../bin/coff2noff halt2.coff halt2

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

cat.o: cat.c
	$(CC) $(CFLAGS) -c cat.c
cat: cat.o start.o
	$(LD) $(LDFLAGS) start.o cat.o -o cat.coff
	../bin/coff2noff cat.coff cat

cp.o: cp.c
	$(CC) $(CFLAGS) -c cp.c
cp: cp.o start.o
	$(LD) $(LDFLAGS) start.o cp.o -o cp.coff
	../bin/coff2noff cp.coff cp

mix_test.o: mix_test.c
	$(CC) $(CFLAGS) -c mix_test.c
mix_test: mix_test.o start.o
	$(LD) $(LDFLAGS) start.o mix_test.o -o mix_test.coff
	../bin/coff2noff mix_test.coff mix_test

mix_test1.o: mix_test1.c
	$(CC) $(CFLAGS) -c mix_test1.c
mix_test1: mix_test1.o start.o
	$(LD) $(LDFLAGS) start.o mix_test1.o -o mix_test1.coff
	../bin/coff2noff mix_test1.coff mix_test1

mix_test2.o: mix_test2.c
	$(CC) $(CFLAGS) -c mix_test2.c
mix_test2: mix_test2.o start.o
	$(LD) $(LDFLAGS) start.o mix_test2.o -o mix_test2.coff
	../bin/coff2noff mix_test2.coff mix_test2

ncat.o: ncat.c
	$(CC) $(CFLAGS) -c ncat.c
ncat: ncat.o start.o
	$(LD) $(LDFLAGS) start.o ncat.o -o ncat.coff
	../bin/coff2noff ncat.coff ncat

org.o: org.c
	$(CC) $(CFLAGS) -c org.c
org: org.o start.o
	$(LD) $(LDFLAGS) start.o org.o -o org.coff
	../bin/coff2noff org.coff org

test1.o: test1.c
	$(CC) $(CFLAGS) -c test1.c
test1: test1.o start.o
	$(LD) $(LDFLAGS) start.o test1.o -o test1.coff
	../bin/coff2noff test1.coff test1

