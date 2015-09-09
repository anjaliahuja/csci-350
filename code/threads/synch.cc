// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
  name = debugName;
  value = initialValue;
  queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
  delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
  IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
  
  while (value == 0) { 			// semaphore not available
    queue->Append((void *)currentThread);	// so go to sleep
    currentThread->Sleep();
  } 
  value--; 					// semaphore available, 
  // consume its value
  
  (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
  Thread *thread;
  IntStatus oldLevel = interrupt->SetLevel(IntOff);

  thread = (Thread *)queue->Remove();
  if (thread != NULL)	   // make thread ready, consuming the V immediately
    scheduler->ReadyToRun(thread);
  value++;
  (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) {
  this.name = debugName;
  this.state = "FREE";
  this.lockOwnerThread = new Thread("test_thread");
  this.lockWaitQueue = new List;
}

Lock::~Lock() {
  delete this.lockWaitQueue;
  delete this.lockOwnerThread;
}


void Lock::Acquire() {
  IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
  //Thread already owns it
  if (currentThread == this.lockOwnerThread) {
    (void) interrupt->SetLevel(oldLevel);
    printf("Lock::Acquire -> Thread %s already owns lock!", this.name);
    return;
  }

  //If the lock is available
  if(this.state == "FREE") {
    this.state = "BUSY";
    this.lockOwnerThread = currentThread;
  } else { //otherwise it's busy
    this.lockWaitQueue->Append((void *)currentThread);	// so go to sleep
    currentThread->Sleep();
  }
  //restore interrupts
  (void) interrupt->SetLevel(oldLevel);
  return;
}

void Lock::Release() {
  IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts

  if (currentThread != this.lockOwnerThread) {
    printf("Lock::Release()-> current Thread %s doesn't own lock!", this.name);
    (void) interrupt->SetLevel(oldLevel);
    return;
  }

  //Thread waiting in the queue
  if(!this.lockWaitQueue.empty()) {
    Thread* removedThread = (Thread *)this.lockWaitQueue->Remove();
    this.lockOwnerThread = removedThread;
    scheduler->ReadyToRun(removedThread);
  } else { //Nothing else
    this.state = "FREE";
    this.lockOwnerThread = NULL;
  }

  (void) interrupt->SetLevel(oldLevel);
  return;
}

bool isHeldByCurrentThread() {
  return currentThread == this.lockOwnerThread;
}

Condition::Condition(char* debugName) { 
  this.name = debugName;
  this.waitingLock = NULL;
  this.waitConditionQueue = new List;
}

Condition::~Condition() {
  delete this.waitingLock;
  delete this.waitConditionQueue;
}

void Condition::Wait(Lock* conditionLock) { 
  IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts

  if(conditionLock == NULL) {
    printf("Condition::Wait -> conditionLock %s , in condition %s is null", conditionLock.getName(), this.getName());
    (void) interrupt->SetLevel(oldLevel);
    return;
  }
  if(this.waitingLock == NULL) {
    this.waitingLock = conditionLock;
  }
  if(this.waitingLock != conditionLock) {
    printf("Condition::Wait -> waitingLock %s, in condition %s doens't equal to ConditionLock %s", this.waitingLock.getName(), this.getName(), conditionLock.getName());
    (void) interrupt->SetLevel(oldLevel);
    return;
  }
  //All three checks are done
  conditionLock->Release();
  this.waitConditionQueue->Append((void *)currentThread);
  currentThread->Sleep();
  conditionLock->Acquire();

  //Not sure what this below does
  //ASSERT(FALSE); 
  
  //Restore Interrupts
  (void) interrupt->SetLevel(oldLevel);
  return;
}

void Condition::Signal(Lock* conditionLock) { 
  IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts

  if(this.waitConditionQueue->empty()) {
    printf("Condition::Signal -> No thread waiting");
    (void) interrupt->SetLevel(oldLevel);
    return;
  }
  if(conditionLock == NULL) {
    printf("Condition::Signal -> conditionLock %s , in condition %s is null", conditionLock.getName(), this.getName());
    (void) interrupt->SetLevel(oldLevel);
    return;
  }
  if(this.waitingLock != conditionLock) {
    printf("Condition::Signal -> conditionLock %s , in condition %s doesn't equal to waitingLock %s", conditionLock.getName(), this.getName(), this.waitingLock.getName();
    (void) interrupt->SetLevel(oldLevel);
    return;
  }

  //All checks done
  Thread* removedThread = (Thread *)this.waitConditionQueue->Remove();
  scheduler->ReadyToRun(removedThread);
  if(this.waitConditionQueue->empty()) {
    this.waitingLock = NULL;
  }
  
  (void) interrupt->SetLevel(oldLevel);
  return;
}

void Condition::Broadcast(Lock* conditionLock) { 
  if(conditionLock == NULL) {
    printf("Condition::Broadcast -> conditionLock %s , in condition %s is null", conditionLock.getName(), this.getName());
    return;
  }
  if(conditionLock != this.waitingLock) {
    printf("Condition::Broadcast -> conditionLock %s , in condition %s doesn't equal to waitingLock %s", conditionLock.getName(), this.getName(), this.waitingLock.getName();
    return;
  }
  while(!waitConditionQueue->empty()) {
    Signal(conditionLock);
  }
}
