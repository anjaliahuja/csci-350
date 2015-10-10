// system.h 
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock

#ifdef USER_PROGRAM
#include "machine.h"
#include "table.h"
#include "bitmap.h"
#include "synch.h"

extern Machine* machine;	// user program memory and registers
extern Lock* availMem; 
extern BitMap* bitMap; 

#define NumLocks 10000
#define NumCVs 10000
#define NumProcesses 20

extern Table* lockTable;
extern Lock* lockTableLock;

extern Table* CVTable;
extern Lock* CVTableLock;

extern Table* processTable;
extern Lock* processLock;

struct kernelLock {
	Lock* lock;
	AddrSpace* addressSpace;
	bool isToBeDeleted;
};

struct kernelCV{
	Condition* condition;
	AddrSpace* addressSpace;
	bool toBeDeleted;
	int cvCounter;
};

struct kernelProcess{
	kernelProcess(){
		addressSpace = currentThread->space; 
		numThreads = 0;
	}
	~kernelProcess(){
	}

	AddrSpace* addressSpace;
	int numThreads;
};

//in lock class make public method as is busy or is available, so if the lock isn't being used, you can go and destroy it, but if the locks busy, dont destroy

//if the lock is busy or if CV is not empty (CVs don't have a state but have a wait queue) if wait queue isnt empty
//Can't destroy it now so you don't want to forget

//in exit system call you go through all locks and CVs and delete everything
//if someone is using it, mark it as 'is to be deleted' when someone releases a lock- after you release- check if its supposed to be deleted
//if its not busy, delete there
//modify original lock and release classes to check if busy
//no semaphores, remove semaphores, because of that (in test suite it guarantees to get the lock first because of sems) without sems
//behavior is different, still can say one of the threads is going to get the lock first and print it out
//certain statements like whoever is first, what you could put- we used -rs37, 37 gave us this output, explain how thats correct 
//you should have a negative test where you create acquire release and DELETE and then maybe try to acquire it after so you show that check works 
//whatever variable you had in project one, use that as the name but now thats an int
//now when you do an acquire, instead of l being there, its between the processes



#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
#endif

#endif // SYSTEM_H
