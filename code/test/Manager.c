#include "syscall.h"
#include "Setup.h"

void managerWakeup(Clerk* clerk, String clerkType) {
  Acquire(clerk->lock);
  Signal(clerk->lock, clerk->cv);
  Printf("Manager has woken up " + clerkType + " %d \n", sizeof("Manager has woken up " + clerkType + " %d \n"), (clerk->id));
  Wait(clerk->lock, clerk->cv);
  Release(clerk->lock);
}

void startManager() {
  int myMV;

  while(true) {

  }
}

int main() {
  setup();
  startManager();
}
