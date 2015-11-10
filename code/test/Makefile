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

all: halt matmult2 matmult sort2 sort testfiles passportsim testexit testexitprogram testprint passporttest1 passporttest2 passporttest3 passporttest4 passporttest5 passporttest6 lock_cv_test1 lock_cv_test2 lock_cv_test3 lock_cv_test4 lock_cv_test5

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

passporttest2.o: passporttest2.c
	$(CC) $(CFLAGS) -c passporttest2.c
passporttest2: passporttest2.o start.o
	$(LD) $(LDFLAGS) start.o passporttest2.o -o passporttest2.coff
	../bin/coff2noff passporttest2.coff passporttest2

lock_cv_test1.o: lock_cv_test1.c
	$(CC) $(CFLAGS) -c lock_cv_test1.c
lock_cv_test1: lock_cv_test1.o start.o
	$(LD) $(LDFLAGS) start.o lock_cv_test1.o -o lock_cv_test1.coff
	../bin/coff2noff lock_cv_test1.coff lock_cv_test1
lock_cv_test2.o: lock_cv_test2.c
	$(CC) $(CFLAGS) -c lock_cv_test2.c
lock_cv_test2: lock_cv_test2.o start.o
	$(LD) $(LDFLAGS) start.o lock_cv_test2.o -o lock_cv_test2.coff
	../bin/coff2noff lock_cv_test2.coff lock_cv_test2
lock_cv_test3.o: lock_cv_test3.c
	$(CC) $(CFLAGS) -c lock_cv_test3.c
lock_cv_test3: lock_cv_test3.o start.o
	$(LD) $(LDFLAGS) start.o lock_cv_test3.o -o lock_cv_test3.coff
	../bin/coff2noff lock_cv_test3.coff lock_cv_test3
lock_cv_test4.o: lock_cv_test4.c
	$(CC) $(CFLAGS) -c lock_cv_test4.c
lock_cv_test4: lock_cv_test4.o start.o
	$(LD) $(LDFLAGS) start.o lock_cv_test4.o -o lock_cv_test4.coff
	../bin/coff2noff lock_cv_test4.coff lock_cv_test4
lock_cv_test5.o: lock_cv_test5.c
	$(CC) $(CFLAGS) -c lock_cv_test5.c
lock_cv_test5: lock_cv_test5.o start.o
	$(LD) $(LDFLAGS) start.o lock_cv_test5.o -o lock_cv_test5.coff
	../bin/coff2noff lock_cv_test5.coff lock_cv_test5

	
passporttest3.o: passporttest3.c
	$(CC) $(CFLAGS) -c passporttest3.c
passporttest3: passporttest3.o start.o
	$(LD) $(LDFLAGS) start.o passporttest3.o -o passporttest3.coff
	../bin/coff2noff passporttest3.coff passporttest3

passporttest4.o: passporttest4.c
	$(CC) $(CFLAGS) -c passporttest4.c
passporttest4: passporttest4.o start.o
	$(LD) $(LDFLAGS) start.o passporttest4.o -o passporttest4.coff
	../bin/coff2noff passporttest4.coff passporttest4

passporttest5.o: passporttest5.c
	$(CC) $(CFLAGS) -c passporttest5.c
passporttest5: passporttest5.o start.o
	$(LD) $(LDFLAGS) start.o passporttest5.o -o passporttest5.coff
	../bin/coff2noff passporttest5.coff passporttest5
passporttest6.o: passporttest6.c
	$(CC) $(CFLAGS) -c passporttest6.c
passporttest6: passporttest6.o start.o
	$(LD) $(LDFLAGS) start.o passporttest6.o -o passporttest6.coff
	../bin/coff2noff passporttest6.coff passporttest6

matmult2.o: matmult2.c
	$(CC) $(CFLAGS) -c matmult2.c
matmult2: matmult2.o start.o
	$(LD) $(LDFLAGS) start.o matmult2.o -o matmult2.coff
	../bin/coff2noff matmult2.coff matmult2
sort2.o: sort2.c
	$(CC) $(CFLAGS) -c sort2.c
sort2: sort2.o start.o
	$(LD) $(LDFLAGS) start.o sort2.o -o sort2.coff
	../bin/coff2noff sort2.coff sort2


clean:
	rm -f *.o *.coff matmult testfiles testexit testexitprogram testprint passporttest1 passporttest2 passporttest3 passporttest4 passporttest5 passporttest6 passportsim
