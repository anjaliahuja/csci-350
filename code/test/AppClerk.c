#include "syscall.h"
#include "Setup.h"

void startAppClerk(){
  int myIndex, myMV; 

  Acquire(DataLock);
  myIndex = NumActiveAppClerks++;
  myMV = GetMV(appClerks, myIndex);
  Release(DataLock);


  while(true); 
  Acquire(GetMV(myMV, ACLock));
  if(GetMV)
}

int main() {
  AppClerkLineLock = CreateLock("AppClerkLineLock", sizeof("AppClerkLineLock"));


  Write("Test 1 acquiring lock\n", sizeof("Test 1 acquiring lock\n"), ConsoleOutput);
  Acquire(lock);
  Write("Test 1 acquires lock\n", sizeof("Test 1 acquires lock\n"), ConsoleOutput);

  for (i = 0; i < 50000; i++) {
    Yield();
  }

  Write("Test 1 releases lock\n", sizeof("Test 1 releases lock\n"), ConsoleOutput);
  Release(lock);

  Exit(0);
}
