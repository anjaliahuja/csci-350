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
	bool isDeleted;
	int lockCounter;
};

struct kernelCV{
	Condition* condition;
	AddrSpace* addressSpace;
	bool isDeleted;
	int cvCounter;
};

struct kernelProcess{
	kernelProcess(){
		addressSpace = currentThread->space; 
		numThreads = 0;
		locks = new bool[NumLocks];
		for(int i = 0; i<NumLocks; i++){
			locks[i]= false;
		}
		cvs = new bool[NumCVs];
		for(int i =0; i<NumCVs; i++){
			cvs[i] = false;
		}
	}
	~kernelProcess(){
		delete [] locks;
		delete [] cvs;
	}
	AddrSpace* addressSpace;
	int numThreads;
	bool* locks;
	bool* cvs; 
};





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
