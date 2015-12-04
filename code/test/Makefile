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

all: halt matmult sort Manager AppClerk AppClerks PicClerk PassportClerk Cashier Customer Customers

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

Manager.o: Manager.c
	$(CC) $(CFLAGS) -c Manager.c
Manager: Manager.o start.o
	$(LD) $(LDFLAGS) start.o Manager.o -o Manager.coff
	../bin/coff2noff Manager.coff Manager

AppClerk.o: AppClerk.c
	$(CC) $(CFLAGS) -c AppClerk.c
AppClerk: AppClerk.o start.o
	$(LD) $(LDFLAGS) start.o AppClerk.o -o AppClerk.coff
	../bin/coff2noff AppClerk.coff AppClerk

AppClerks.o: AppClerks.c
	$(CC) $(CFLAGS) -c AppClerks.c
AppClerks: AppClerks.o start.o
	$(LD) $(LDFLAGS) start.o AppClerks.o -o AppClerks.coff
	../bin/coff2noff AppClerks.coff AppClerks

PicClerk.o: PicClerk.c
	$(CC) $(CFLAGS) -c PicClerk.c
PicClerk: PicClerk.o start.o
	$(LD) $(LDFLAGS) start.o PicClerk.o -o PicClerk.coff
	../bin/coff2noff PicClerk.coff PicClerk

PassportClerk.o: PassportClerk.c
	$(CC) $(CFLAGS) -c PassportClerk.c
PassportClerk: PassportClerk.o start.o
	$(LD) $(LDFLAGS) start.o PassportClerk.o -o PassportClerk.coff
	../bin/coff2noff PassportClerk.coff PassportClerk

Cashier.o: Cashier.c
	$(CC) $(CFLAGS) -c Cashier.c
Cashier: Cashier.o start.o
	$(LD) $(LDFLAGS) start.o Cashier.o -o Cashier.coff
	../bin/coff2noff Cashier.coff Cashier

Customer.o: Customer.c
	$(CC) $(CFLAGS) -c Customer.c
Customer: Customer.o start.o
	$(LD) $(LDFLAGS) start.o Customer.o -o Customer.coff
	../bin/coff2noff Customer.coff Customer

Customers.o: Customers.c
	$(CC) $(CFLAGS) -c Customers.c
Customers: Customers.o start.o
	$(LD) $(LDFLAGS) start.o Customers.o -o Customers.coff
	../bin/coff2noff Customers.coff Customers

clean:
	rm -f *.o *.coff halt matmult sort Manager AppClerk AppClerks PicClerk PassportClerk Cashier Customer Customers
