// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <stdio.h>
#include <iostream>

using namespace std;

int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n=0;			// The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
      result = machine->ReadMem( vaddr, 1, paddr );
      while(!result) // FALL 09 CHANGES
	  {
   			result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
	  }	
      
      buf[n++] = *paddr;
     
      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    delete paddr;
    return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occors.  Errors can generally mean a bad virtual address was
    // passed in.
    bool result;
    int n=0;			// The number of bytes copied in

    while ( n >= 0 && n < len) {
      // Note that we check every byte's address
      result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Create\n");
	delete buf;
	return;
    }

    buf[len]='\0';

    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file
    int id;				// The openfile id

    if (!buf) {
	printf("%s","Can't allocate kernel buffer in Open\n");
	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Open\n");
	delete[] buf;
	return -1;
    }

    buf[len]='\0';

    f = fileSystem->Open(buf);
    delete[] buf;

    if ( f ) {
	if ((id = currentThread->space->fileTable.Put(f)) == -1 )
	    delete f;
	return id;
    }
    else
	return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.
    
    char *buf;		// Kernel buffer for output
    OpenFile *f;	// Open file for output

    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer for write!\n");
	return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
	    printf("%s","Bad pointer passed to to write: data not written\n");
	    delete[] buf;
	    return;
	}
    }

    if ( id == ConsoleOutput) {
      for (int ii=0; ii<len; ii++) {
	printf("%c",buf[ii]);
      }

    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    f->Write(buf, len);
	} else {
	    printf("%s","Bad OpenFileId passed to Write\n");
	    len = -1;
	}
    }

    delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;		// Kernel buffer for input
    OpenFile *f;	// Open file for output

    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer in Read\n");
	return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf("%s", buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
	printf("%s","Bad pointer passed to Read: data not copied\n");
      }
    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    len = f->Read(buf, len);
	    if ( len > 0 ) {
	        //Read something from the file. Put into user's address space
  	        if ( copyout(vaddr, len, buf) == -1 ) {
		    printf("%s","Bad pointer passed to Read: data not copied\n");
		}
	    }
	} else {
	    printf("%s","Bad OpenFileId passed to Read\n");
	    len = -1;
	}
    }

    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

    if ( f ) {
      delete f;
    } else {
      printf("%s","Tried to close an unopen file\n");
    }
}
int CreateCV_Syscall(int vaddr, int size) {
  CVTableLock->Acquire();
  printf("Creating a CV");

  char* buf;
  if(!(buf = new char[size])){
    printf("Create CV:Error allocating kernel buffer for Fork \n");
    return -1;
  }
  else {
    if(copyin(vaddr, size, buf)==-1){
      printf("Create CV: Bad pointer passed to fork call, thread not forked \n");
      delete[] buf;
      return -1;
    }
  }
  //Error checking
  //If CVTable is full, then don't create new CV
  if(CVTable->NumUsed() >= NumCVs) {
      printf("Create CV: Condition Variable Table is full 1, couldn't create Condition");
      CVTableLock->Release();
      return -1;
  }

  //Creating condition
  Condition * cv = new Condition(buf);
  kernelCV* kcv = new kernelCV();
  kcv->condition = cv;
  kcv->toBeDeleted = false;
  kcv->addressSpace = currentThread->space;
  kcv->cvCounter = 0;

  int index = CVTable->Put((void*) kcv);
  if (index == -1) {
    printf("Create CV: Condition Variable Table is full 2, couldn't create Condition");
  }

  CVTableLock->Release();
  return index;
}

int DestroyCV_Syscall(int index) {
  CVTableLock->Acquire();
  printf("Destroying a CV");
  //Error checking
  if (index < 0 || index > NumCVs) {
    printf("Destroy CV: Invalid Index");
    CVTableLock->Release();
    return -1;
  }

  kernelCV* kcv = (kernelCV*)CVTable->Get(index);

  if (kcv == NULL || kcv->condition == NULL) {
    printf("Destroy CV: CV does not exist");
    CVTableLock->Release();
    return -1;
  }

  if (kcv->addressSpace != currentThread->space) {
    printf("DestroyCV: CV belongs to a different process");
    CVTableLock->Release();
    return -1;
  }

  //Destryoing condition
  if (kcv->cvCounter == 0){
    kcv = (kernelCV*) CVTable->Remove(index);
    delete kcv->condition;
    delete kcv;
  } else {
    kcv->toBeDeleted = true;
  }

  CVTableLock->Release();
  return index;
}

int Wait_Syscall(int lockIndex, int CVIndex) {
  CVTableLock->Acquire();
  printf("Wait CV: Lock index %d amd CV index %d are in wait", lockIndex, CVIndex);

  //Error checking
  //validating lock
  if (lockIndex < 0 || lockIndex > NumLocks) {
    printf("Wait CV: Invalid Lock Index");
    CVTableLock->Release();
    return -1;
  }

  kernelLock* kl = (kernelLock*)lockTable->Get(lockIndex);

  if (kl == NULL) {
    printf("Wait CV: CV does not exist");
    CVTableLock->Release();
    return -1;
  }

  if (kl->addressSpace != currentThread->space) {
    printf("WaitCV: CV belongs to a different process");
    CVTableLock->Release();
    return -1;
  }

  //validating cv
  if (CVIndex < 0 || CVIndex > NumCVs) {
    printf("Wait CV: Invalid Lock Index");
    CVTableLock->Release();
    return -1;
  }

  kernelCV* kcv = (kernelCV*)CVTable->Get(CVIndex);

  if (kcv == NULL || kcv->condition == NULL) {
    printf("Wait CV: CV does not exist");
    CVTableLock->Release();
    return -1;
  }

  if (kcv->addressSpace != currentThread->space) {
    printf("WaitCV: CV belongs to a different process");
    CVTableLock->Release();
    return -1;
  }
  //Wait
  kcv->cvCounter++;
  CVTableLock->Release();
  kcv->condition->Wait(kl->lock);
  CVTableLock->Acquire();
  kcv->cvCounter;

  //Check if needs to be deleted
  if (kcv->toBeDeleted == true && kcv->cvCounter == 0) {
    printf("Wait CV: Condition is deleted");
    kcv = (kernelCV*) CVTable->Remove(CVIndex);
    delete kcv->condition;
    delete kcv;
  }

  CVTableLock->Release();
  return CVIndex;
}

int Signal_Syscall(int lockIndex, int CVIndex) {
  CVTableLock->Acquire();
  //Error checking
  printf("Signal CV: Lock index %d amd CV index %d are in wait", lockIndex, CVIndex);

  //Error checking
  //validating lock
  if (lockIndex < 0 || lockIndex > NumLocks) {
    printf("Wait CV: Invalid Lock Index");
    CVTableLock->Release();
    return -1;
  }

  kernelLock* kl = (kernelLock*)lockTable->Get(lockIndex);

  if (kl == NULL) {
    printf("Wait CV: CV does not exist");
    CVTableLock->Release();
    return -1;
  }

  if (kl->addressSpace != currentThread->space) {
    printf("WaitCV: CV belongs to a different process");
    CVTableLock->Release();
    return -1;
  }

  //validating cv
  if (CVIndex < 0 || CVIndex > NumCVs) {
    printf("Wait CV: Invalid Lock Index");
    CVTableLock->Release();
    return -1;
  }

  kernelCV* kcv = (kernelCV*)CVTable->Get(CVIndex);

  if (kcv == NULL || kcv->condition == NULL) {
    printf("Wait CV: CV does not exist");
    CVTableLock->Release();
    return -1;
  }

  if (kcv->addressSpace != currentThread->space) {
    printf("WaitCV: CV belongs to a different process");
    CVTableLock->Release();
    return -1;
  }
  //Signal
  kcv->condition->Signal(kl->lock);
  CVTableLock->Release();
  return CVIndex;
}

int Broadcast_Syscall(int lockIndex, int CVIndex) {
  CVTableLock->Acquire();
  //Error checking
  printf("Broadcast CV: Lock index %d amd CV index %d are in wait", lockIndex, CVIndex);

  //Error checking
  //validating lock
  if (lockIndex < 0 || lockIndex > NumLocks) {
    printf("Wait CV: Invalid Lock Index");
    CVTableLock->Release();
    return -1;
  }

  kernelLock* kl = (kernelLock*)lockTable->Get(lockIndex);

  if (kl == NULL) {
    printf("Wait CV: CV does not exist");
    CVTableLock->Release();
    return -1;
  }

  if (kl->addressSpace != currentThread->space) {
    printf("WaitCV: CV belongs to a different process");
    CVTableLock->Release();
    return -1;
  }

  //validating cv
  if (CVIndex < 0 || CVIndex > NumCVs) {
    printf("Wait CV: Invalid Lock Index");
    CVTableLock->Release();
    return -1;
  }

  kernelCV* kcv = (kernelCV*)CVTable->Get(CVIndex);

  if (kcv == NULL || kcv->condition == NULL) {
    printf("Wait CV: CV does not exist");
    CVTableLock->Release();
    return -1;
  }

  if (kcv->addressSpace != currentThread->space) {
    printf("WaitCV: CV belongs to a different process");
    CVTableLock->Release();
    return -1;
  }
  //Broadcast
  kcv->condition->Broadcast(kl->lock);
  CVTableLock->Release();
  return CVIndex;
}

int CreateLock_Syscall(unsigned int vaddr, int len) {
  lockTableLock->Acquire();

  // Setting up name
  char* name;
  if(!(name = new char[len])){
    printf("CreateLock: Can't allocate kernel buffer for exec system call\n");
    lockTableLock->Release();
    return -1;
  }
  else {
    if(copyin(vaddr, len, name) == -1){
      printf("CreateLock: Bad pointer passed to write\n");
      delete [] name;
      lockTableLock->Release();
      return -1;
    }
  }

  // Create lock
  kernelLock* kl = new kernelLock();
  kl->lock = new Lock(name);
  kl->addressSpace = currentThread->space;
  kl->isToBeDeleted = false;

  int index = lockTable->Put((void*)kl);

  // Error checking
  if (index == -1) {
    printf("CreateLock: No space left in lock table to create lock\n"); 
    lockTableLock->Release();
    return -1;
  }

  lockTableLock->Release();

  return index;
}

int Acquire_Syscall(int index) {
  lockTableLock->Acquire();

  // Error checking
  // index falls within range of table size
  if (index < 0 || index > NumLocks) {
    printf("Acquire: Invalid index\n");
    lockTableLock->Release();
    return -1;
  }

  kernelLock* kl = (kernelLock*)lockTable->Get(index);

  // does lock actually exist at this index
  if (kl->lock == NULL) {
    printf("Acquire: Lock does not exist\n");
    lockTableLock->Release();
    return -1;
  }

  // does thread belong to same process as thread creator
  if (kl->addressSpace != currentThread->space) {
    printf("Acquire: Lock belongs to a different process\n");
    lockTableLock->Release();
    return -1;
  }

  kl->lock->Acquire();

  lockTableLock->Release();
  return index;
}

int Release_Syscall(int index) {
  lockTableLock->Acquire();

  // Error checking
  // index falls within range of table size
  if (index < 0 || index > NumLocks) {
    printf("Release: Invalid index\n");
    lockTableLock->Release();
    return -1;
  }

  kernelLock* kl = (kernelLock*)lockTable->Get(index);

  // does lock actually exist at this index
  if (kl->lock == NULL) {
    printf("Release: Lock does not exist\n");
    lockTableLock->Release();
    return -1;
  }

  // does thread belong to same process as thread creator
  if (kl->addressSpace != currentThread->space) {
    printf("Release: Lock belongs to a different process\n");
    lockTableLock->Release();
    return -1;
  }

  kl->lock->Release();

  // Delete lock if destroyed was called on it previouisly and no threads 
  // are waiting
  if (kl->isToBeDeleted && strcmp(kl->lock->getState(), "FREE")) {
    kl = (kernelLock*) lockTable->Remove(index);
    delete kl;
  }

  lockTableLock->Release();
  return index;
}

int DestroyLock_Syscall(int index) {
  lockTableLock->Acquire();

  // Error checking
  // index falls within range of table size
  if (index < 0 || index > NumLocks) {
    printf("DestroyLock: Invalid index\n");
    lockTableLock->Release();
    return -1;
  }

  kernelLock* kl = (kernelLock*)lockTable->Get(index);

  // does lock actually exist at this index
  if (kl->lock == NULL) {
    printf("DestroyLock: Lock does not exist\n");
    lockTableLock->Release();
    return -1;
  }

  // does thread belong to same process as thread creator
  if (kl->addressSpace != currentThread->space) {
    printf("DestroyLock: Lock belongs to a different process\n");
    lockTableLock->Release();
    return -1;
  }

  // if lock is busy, delete after it gets released
  if (strcmp(kl->lock->getState(), "BUSY")) {
    DEBUG('a', "DestroyLock: Lock is busy, set isToBeDeleted to true\n");
    kl->isToBeDeleted = true;
    lockTableLock->Release();
    return index;
  }

  kl = (kernelLock*) lockTable->Remove(index);
  delete kl;

  lockTableLock->Release();
  return index;
}

void internal_fork(int pc){
  currentThread->space->InitRegisters();
  machine->WriteRegister(PCReg, pc);
  machine->WriteRegister(NextPCReg, pc+4);
  machine->WriteRegister(StackReg, currentThread->stackreg);
  currentThread->space->RestoreState();

  machine->Run();
}

void Fork_Syscall(int pc, unsigned int vaddr, int len){
  char* buf;
  if(!(buf = new char[len])){
    printf("Error allocating kernel buffer for Fork \n");
    return;
  }
  else {
    if(copyin(vaddr, len, buf)==-1){
      printf("Bad pointer passed to fork call, thread not forked \n");
      delete[] buf;
      return;
    }
  }


  processLock->Acquire();
  int processID = -1;
  kernelProcess* process;
  for(int i = 0; i<NumProcesses; i++){
    process =(kernelProcess*) processTable ->Get(i);
    if(process==NULL){
      continue;
    }
    if(process->addressSpace == currentThread->space){
      processID = i;
      break;
    }
  }
  if(processID == -1){
    printf("Invalid process identified");
    processLock->Release();
    return;
  }

  processLock->Release();

    if(pc == 0){
    printf("Null pointer passed in to Fork. Can't fork\n");
  }



  //Fork new thread
  Thread* t = new Thread(buf);
  //Allocate stack
  int* stackRegister = currentThread->space->AllocateStack();
  t->stackreg = stackRegister[0];
  t->stackVP = stackRegister[1];
  delete[] stackRegister;
  t->space = currentThread->space;

  t->Fork((VoidFunctionPtr)internal_fork, pc);
}

void internal_exec(int pc){
  currentThread->space->InitRegisters();
  currentThread->space->RestoreState();
  machine->Run();
}

void Exec_Syscall(unsigned int vaddr, int len){
  char* file;
  if(!(file = new char[len])){
    printf("Can't allocate kernel buffer for exec system call");
    return;
  }
  else {
    if(copyin(vaddr, len, file) == -1){
      printf("Bad pointer passed to write");
      delete [] file;
      return;
    }
  }

  OpenFile* exec = fileSystem->Open(file);
  AddrSpace* addressSpace;

  if (exec == NULL){
    printf("file is null");
    return;
  }


  //Allocate address space and fork a new thread
  Thread* t = new Thread(file);
  availMem->Acquire();
    addressSpace = new AddrSpace(exec);
    t->stackVP = addressSpace->numPages-1;
  availMem->Release();
  t->space = addressSpace; 


  //Add process to process table
  kernelProcess* process = new kernelProcess();
  processLock->Acquire();
  process->addressSpace = addressSpace;
  process->numThreads++;
  processLock->Release();
  int index = processTable->Put((void*)process);
  if(index == -1){
    printf("Invalid, no space left in process table to fork");
    return;
  }
  t->Fork((VoidFunctionPtr)internal_exec,0);  


}


void Yield_Syscall(){
  currentThread->Yield();
} 
void Exit_Syscall(int status){
  processLock->Acquire();

  bool lastProcess = false;
  if(processTable->NumUsed() == 1){ // only one process in table, its the last process
    lastProcess = true;
  }

  //Find current process

  int processID = -1;
  kernelProcess* process;
  for(int i =0; i<NumProcesses; i++){
    process = (kernelProcess*) processTable->Get(i);
    if(process == NULL){
      continue; 
    }
    if(process->addressSpace == currentThread->space){
      processID = i;
      break;
    }
  }
  if(processID == -1){
    printf("Invalid process identifier");
    processLock->Release();
    return;
  }
  //3 Cases
  //Case 1: Check if thread has called exit in a process but its not the last thread
  //Reclaim 8 pages 

  if(process->numThreads > 1){
    availMem->Acquire();
    int pageNum = currentThread->stackVP;
    for(int i = 0; i < 8; i++){
      bitMap->Clear(currentThread->space->pageTable[pageNum].physicalPage);
      currentThread->space->pageTable[pageNum].valid = FALSE;
      pageNum--;
    }
    availMem->Release(); 
    process->numThreads--;
    DEBUG('b', "Exit thread case 1\n");
  }

  //Case 2: last executing thread in last process (ready queue is empty)
  else if(lastProcess && process->numThreads == 1){
    availMem->Acquire();
    for(unsigned int i =0; i<currentThread->space->numPages; i++){
      bitMap->Clear(currentThread->space->pageTable[i].physicalPage);
      currentThread->space->pageTable[i].valid = FALSE;
    }
    availMem->Release();

     currentThread->Finish();
    processLock->Release();
    interrupt->Halt();
  }

  //Case 3: Last thread in process but not last process, need to reclaim all locks, cvs, stack memory
  else if(!lastProcess && process->numThreads == 1){
      //Delete CVs

      availMem->Acquire();
        for(unsigned int i=0; i< currentThread->space->numPages; i++){
          if(currentThread->space->pageTable[i].valid){
            bitMap->Clear(currentThread->space->pageTable[i].physicalPage);
            currentThread->space->pageTable[i].valid= FALSE;
          }
        }
      availMem->Release();
      
      //Delete CVs
      for(int i =0; i<NumCVs; i++){
        DestroyCV_Syscall(i);
        }

      //Delete Locks
        for(int i =0; i<NumLocks; i++){
          DestroyLock_Syscall(i);
            }
          


        processTable->Remove(processID);
        delete process;

      }
   
  else{
    printf("Invalid case");
  }

  processLock->Release();
  currentThread->Finish();

}

//Implementation for CVs


void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv=0; 	// the return value from a syscall

    if ( which == SyscallException ) {
	switch (type) {
	    default:
		DEBUG('a', "Unknown syscall - shutting down.\n");
	 case SC_Halt:
    		DEBUG('a', "Shutdown, initiated by user program.\n");
    		interrupt->Halt();
    		break;
	 case SC_Create:
    		DEBUG('a', "Create syscall.\n");
    		Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
    		break;
	 case SC_Open:
    		DEBUG('a', "Open syscall.\n");
    		rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
    		break;
	 case SC_Write:
    		DEBUG('a', "Write syscall.\n");
    		Write_Syscall(machine->ReadRegister(4),
    			      machine->ReadRegister(5),
    			      machine->ReadRegister(6));
    		break;
	  case SC_Read:
    		DEBUG('a', "Read syscall.\n");
    		rv = Read_Syscall(machine->ReadRegister(4),
    			      machine->ReadRegister(5),
    			      machine->ReadRegister(6));
    		break;
	  case SC_Close:
    		DEBUG('a', "Close syscall.\n");
    		Close_Syscall(machine->ReadRegister(4));
    		break;
    case SC_CreateLock:
        DEBUG('a', "Create lock syscall.\n");
        CreateLock_Syscall(machine->ReadRegister(4),
                           machine->ReadRegister(5));
        break;
    case SC_Acquire:
        DEBUG('a', "Create acquire syscall.\n");
        Acquire_Syscall(machine->ReadRegister(4));
        break;
    case SC_Release:
        DEBUG('a', "Create release syscall.\n");
        Release_Syscall(machine->ReadRegister(4));
        break;
    case SC_DestroyLock:
        DEBUG('a', "Create destroy lock syscall.\n");
        DestroyLock_Syscall(machine->ReadRegister(4));
        break;
    case SC_CreateCV:
        DEBUG('a', "CreateCV syscall.\n");
        rv = CreateCV_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;
    case SC_DestroyCV:
        DEBUG('a', "DestroyCV syscall.\n");
        rv = DestroyCV_Syscall(machine->ReadRegister(4));
        break;
    case SC_Wait:
        DEBUG('a', "Wait syscall.\n");
        rv = Wait_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;
    case SC_Signal:
        DEBUG('a', "Signal syscall.\n");
        rv = Signal_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;
    case SC_Broadcast:
        DEBUG('a', "Broadcast syscall.\n");
        rv = Broadcast_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;
    case SC_Fork:
        DEBUG('a', "Fork syscall.\n");
        Fork_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
        break;
    case SC_Yield:
        DEBUG('a', "Yield syscall.\n");
        Yield_Syscall();
        break;
    case SC_Exec:
        DEBUG('a', "Exec syscall.\n");
        Exec_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;
    case SC_Exit:
        DEBUG('a', "Exit syscall.\n");
        Exit_Syscall(machine->ReadRegister(4));
        break;

	}

	// Put in the return value and increment the PC
	machine->WriteRegister(2,rv);
	machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
	machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
	machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
	return;
    } else {
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
}
