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

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = ../gnu/
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: halt matmult sort testfiles passportsim passportstart testexit testexitprogram testprint passporttest1

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

testfiles.o: testfiles.c
	$(CC) $(CFLAGS) -c testfiles.c
testfiles: testfiles.o start.o
	$(LD) $(LDFLAGS) start.o testfiles.o -o testfiles.coff
	../bin/coff2noff testfiles.coff testfiles

passportsim.o: passportsim.c
	$(CC) $(CFLAGS) -c passportsim.c
passportsim: passportsim.o start.o
	$(LD) $(LDFLAGS) start.o passportsim.o -o passportsim.coff
	../bin/coff2noff passportsim.coff passportsim
passportstart.o: passportstart.c
	$(CC) $(CFLAGS) -c passportstart.c
passportstart: passportstart.o start.o
	$(LD) $(LDFLAGS) start.o passportstart.o -o passportstart.coff
	../bin/coff2noff passportstart.coff passportstart
testexit.o: testexit.c
	$(CC) $(CFLAGS) -c testexit.c
testexit: testexit.o start.o
	$(LD) $(LDFLAGS) start.o testexit.o -o testexit.coff
	../bin/coff2noff testexit.coff testexit
testexitprogram.o: testexitprogram.c
	$(CC) $(CFLAGS) -c testexitprogram.c
testexitprogram: testexitprogram.o start.o
	$(LD) $(LDFLAGS) start.o testexitprogram.o -o testexitprogram.coff
	../bin/coff2noff testexitprogram.coff testexitprogram
testprint.o: testprint.c
	$(CC) $(CFLAGS) -c testprint.c
testprint: testprint.o start.o
	$(LD) $(LDFLAGS) start.o testprint.o -o testprint.coff
	../bin/coff2noff testprint.coff testprint
passporttest1.o: passporttest1.c
	$(CC) $(CFLAGS) -c passporttest1.c
passporttest1: passporttest1.o start.o
	$(LD) $(LDFLAGS) start.o passporttest1.o -o passporttest1.coff
	../bin/coff2noff passporttest1.coff passporttest1
clean:
	rm -f *.o *.coff matmult testfiles testexit testexitprogram testprint
