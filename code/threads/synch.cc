// synch.cc 
//  Routines for synchronizing threads.  Three kinds of
//  synchronization routines are defined here: semaphores, locks 
//    and condition variables (the implementation of the last two
//  are left to the reader).
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
//  Initialize a semaphore, so that it can be used for synchronization.
//
//  "debugName" is an arbitrary name, useful for debugging.
//  "initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
  name = debugName;
  value = initialValue;
  queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
//  De-allocate semaphore, when no longer needed.  Assume no one
//  is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
  delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
//  Wait until semaphore value > 0, then decrement.  Checking the
//  value and decrementing must be done atomically, so we
//  need to disable interrupts before checking the value.
//
//  Note that Thread::Sleep assumes that interrupts are disabled
//  when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
  IntStatus oldLevel = interrupt->SetLevel(IntOff); // disable interrupts
  
  while (value == 0) {      // semaphore not available
    queue->Append((void *)currentThread); // so go to sleep
    currentThread->Sleep();
  } 
  value--;          // semaphore available, 
  // consume its value
  
  (void) interrupt->SetLevel(oldLevel); // re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
//  Increment semaphore value, waking up a waiter if necessary.
//  As with P(), this operation must be atomic, so we need to disable
//  interrupts.  Scheduler::ReadyToRun() assumes that threads
//  are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
  Thread *thread;
  IntStatus oldLevel = interrupt->SetLevel(IntOff);

  thread = (Thread *)queue->Remove();
  if (thread != NULL)    // make thread ready, consuming the V immediately
    scheduler->ReadyToRun(thread);
  value++;
  (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) {
  this->name = debugName;
  this->state = "FREE";
  this->lockOwnerThread = NULL;
  this->lockWaitQueue = new List;
}

Lock::~Lock() {
  delete this->lockWaitQueue;
  delete this->lockOwnerThread;
}


bool Lock::Acquire() {
  // Disable interrupts.
  IntStatus oldLevel = interrupt->SetLevel(IntOff); 
  
  // If thread already owns lock, throw error.
  if (this->isHeldByCurrentThread()) {
    (void) interrupt->SetLevel(oldLevel);
    printf("Lock::Acquire -> Thread %s already owns lock %s!\n", 
      this->lockOwnerThread->getName(), this->name);
    return false;
  }  

  // If the lock is available change to busy.
  // Make currentThread owner.
  if(!strcmp(this->state, "FREE")) {
    this->state = "BUSY";
    this->lockOwnerThread = currentThread;
  // Otherwise it's busy.
  // Put current thread on Lock's wait queue & put thread to sleep.
  } else { 
    this->lockWaitQueue->Append((void *)currentThread);
    currentThread->Sleep();
  }
  //Restore interrupts.
  (void) interrupt->SetLevel(oldLevel);
  return true;
}

bool Lock::Release() {
  // Disable interrupts.
  IntStatus oldLevel = interrupt->SetLevel(IntOff);

  // If thread does not own lock, throw error.
  if (!this->isHeldByCurrentThread()) {
    printf("Lock::Release -> current Thread %s doesn't own lock %s\n", 
      currentThread->getName(), this->name);
    (void) interrupt->SetLevel(oldLevel);
    return false;
  }

  // If there is a thread in the wait queue, remove the thread.
  // Make thread the lock owner and put on ready queue.
  if(!this->lockWaitQueue->IsEmpty()) {
    Thread* removedThread = (Thread *)(this->lockWaitQueue->Remove());
    this->lockOwnerThread = removedThread;
    scheduler->ReadyToRun(removedThread);
  // If there are no threads, the lock is available/free.
  } else {
    this->state = "FREE";
    this->lockOwnerThread = NULL;
  }

  // Restore interrupts.
  (void) interrupt->SetLevel(oldLevel);
  return true;
}

bool Lock::isHeldByCurrentThread() {
  return currentThread == this->lockOwnerThread;
}

Condition::Condition(char* debugName) {
  this->name = debugName;
  this->waitingLock = NULL;
  this->waitConditionQueue = new List;
}

Condition::~Condition() {
  delete this->waitingLock;
  delete this->waitConditionQueue;
}

bool Condition::Wait(Lock* conditionLock) { 
  // Disable interrupts.
  IntStatus oldLevel = interrupt->SetLevel(IntOff);

  // If conditionLock is null, throw error.
  if(conditionLock == NULL) {
    printf("Condition::Wait -> conditionLock %s , in condition %s, is null\n", 
      conditionLock->getName(), this->name);
    (void) interrupt->SetLevel(oldLevel);
    return false;
  }

  // If waitingLock is null, set to conditionLock.
  if(this->waitingLock == NULL) {
    this->waitingLock = conditionLock;
  }

  // If waitingLock and conditionLock are not the same, they are on different 
  // critical sections, throw error.
  if(this->waitingLock != conditionLock) {
    printf("Condition::Wait -> waitingLock %s, in condition %s, doesn't equal conditionLock %s\n", 
      this->waitingLock->getName(), this->name, conditionLock->getName());
    (void) interrupt->SetLevel(oldLevel);
    return false;
  }

  // Release lock, add currentThread to waitQueue and sleep.
  conditionLock->Release();
  this->waitConditionQueue->Append((void *)currentThread);
  currentThread->Sleep();
  conditionLock->Acquire();

  //Not sure what this below does.
  //ASSERT(FALSE); 
  
  //Restore interrupts.
  (void) interrupt->SetLevel(oldLevel);
  return true;
}

bool Condition::Signal(Lock* conditionLock) { 
  // Disable interrupts.
  IntStatus oldLevel = interrupt->SetLevel(IntOff);

  // If the wait queue is empty, there is no thread to signal, throw error.
  if(this->waitConditionQueue->IsEmpty()) {

    (void) interrupt->SetLevel(oldLevel);
    return false;
  }

  // If conditionLock is null, throw error.
  if(conditionLock == NULL) {
    printf("Condition::Signal -> conditionLock %s, in condition %s, is null\n", 
      conditionLock->getName(), this->name);
    (void) interrupt->SetLevel(oldLevel);
    return false;
  }

  // If waitingLock and conditionLock are not the same, they are on different 
  // critical sections, throw error.
  if(this->waitingLock != conditionLock) {
    printf("Condition::Signal -> waitingLock %s , in condition %s, doesn't equal conditionLock %s\n", 
      this->waitingLock->getName(), this->name, conditionLock->getName());
    (void) interrupt->SetLevel(oldLevel);
    return false;
  }

  // Remove/wakeup thread and place on wait queue.
  Thread* removedThread = (Thread *)this->waitConditionQueue->Remove();
  scheduler->ReadyToRun(removedThread);

  // If there are no threads left on wait queue, set waitingLock to null.
  if(this->waitConditionQueue->IsEmpty()) {
    this->waitingLock = NULL;
  }
  
  // Restore interrupts.
  (void) interrupt->SetLevel(oldLevel);
  return true;
}

void Condition::Broadcast(Lock* conditionLock) { 
  // If conditionLock is null, throw error.
  if(conditionLock == NULL) {
    printf("Condition::Broadcast -> conditionLock %s, in condition %s, is null\n", 
      conditionLock->getName(), this->name);
    return;
  }

  // If waitingLock and conditionLock are not the same, they are on different 
  // critical sections, throw error.
  if(this->waitingLock != conditionLock) {
    printf("Condition::Broadcast -> waitingLock %s , in condition %s, doesn't equal conditionLock %s\n", 
      this->waitingLock->getName(), this->name, conditionLock->getName());
    return;
  }

  // Signal each thread until empty.
  while(!waitConditionQueue->IsEmpty()) {
    Signal(conditionLock);
  }
}
