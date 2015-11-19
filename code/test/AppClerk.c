#include "syscall.h"
#include "Setup.h"

char* name;
int id;
int state;
int lock;
int cv;
int lineCV;
int bribeLineCV;
int senatorCV;

int currentCustomer;
Queue line;
Queue bribeLine;

int AppClerkLineLock;
int AppClerkBribeMoney;
int numActiveAppClerks;


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
