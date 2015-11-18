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
#include "list.h"

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
extern int netname;	

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
	bool toBeDeleted;
	int lockCounter;
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


struct InvertedPageTable : public TranslationEntry {
	AddrSpace* addressSpace;
};
enum PageReplacementPolicy {
	RAND,
	FIFO
};

extern InvertedPageTable* ipt;
extern PageReplacementPolicy pageReplacementPolicy;
extern List* iptQueue;
extern Lock* iptLock;


#include "filesys.h"
#define SwapSize 5000
extern OpenFile* swapfile;
extern BitMap* swapMap; 

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
#define SERVER_ID 0
#define MAX_MV 1000

#define RPC_CreateLock   1
#define RPC_DestroyLock  2
#define RPC_Acquire      3
#define RPC_Release      4
#define RPC_CreateCV     5
#define RPC_DestroyCV    6
#define RPC_Wait         7
#define RPC_Signal       8
#define RPC_Broadcast    9
#define RPC_CreateMV	 10
#define RPC_GetMV  	     11
#define RPC_SetMV 		 12
#define RPC_DestroyMV	 13

#endif

#endif // SYSTEM_H
