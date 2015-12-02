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


#define RPC_Server_CreateLock   111
#define RPC_Server_DestroyLock  112
#define RPC_Server_Acquire      113
#define RPC_Server_Release      114
#define RPC_Server_CreateCV     115
#define RPC_Server_DestroyCV    116
#define RPC_Server_Wait1        117
#define RPC_Server_Wait2        118
#define RPC_Server_Wait3        119
#define RPC_Server_Signal1      120
#define RPC_Server_Signal2      121
#define RPC_Server_Signal3      122


#define RPC_Server_Broadcast1    123
#define RPC_Server_Broadcast2    124
#define RPC_Server_Broadcast3    125
#define RPC_Server_CreateMV	 	126
#define RPC_Server_GetMV  	     127
#define RPC_Server_SetMV 		 128
#define RPC_Server_DestroyMV	 129



#define RPC_ServerReply_CreateLock   211
#define RPC_ServerReply_DestroyLock  212
#define RPC_ServerReply_Acquire      213
#define RPC_ServerReply_Release      214
#define RPC_ServerReply_CreateCV     215
#define RPC_ServerReply_DestroyCV    216
#define RPC_ServerReply_Wait1        217
#define RPC_ServerReply_Wait2        218
#define RPC_ServerReply_Wait3        219
#define RPC_ServerReply_Signal1      220
#define RPC_ServerReply_Signal2      221
#define RPC_ServerReply_Signal3      222


#define RPC_ServerReply_Broadcast1    223
#define RPC_ServerReply_Broadcast2    224
#define RPC_ServerReply_Broadcast3    225
#define RPC_ServerReply_CreateMV	 	226
#define RPC_ServerReply_GetMV  	     227
#define RPC_ServerReply_SetMV 		 228
#define RPC_ServerReply_DestroyMV	 229




#endif

#endif // SYSTEM_H
