#include "syscall.h"
#include "Setup.h"

void managerWakeup(int clerk, String clerkType) {
  int lock, cv;
  lock = GetMV(clerk, 2);
  cv = GetMV(clerk, 3);

  Acquire(lock);
  Signal(lock, cv);
  Printf("Manager has woken up " + clerkType + " %d \n", sizeof("Manager has woken up " + clerkType + " %d \n"), (clerk->id));
  Wait(lock, cv);
  Release(lock);
}

void startManager() {
  int i, j, currClerkI, currClerkJ, lineSizeI, lineSizeJ, state;


  while(true) {
    for(i = 0; i < NUM_APPCLERKS; i++) {
      /*If the clerk is on break, aka their state is 2 and their line has more than 3 people
      Wake up the thread*/
      currClerkI = GetMV(appClerks, i);
      lineSizeI = GetMV(currClerkI, ACCount)
      if(lineSizeI > 2) {
          for(j = 0; j < NUM_APPCLERKS; j++) {
            currClerkJ = GetMV(appClerks, j);
            state = GetMV(currClerkJ, ACState);

            if(state == ONBREAK) {
              managerWakeup(currClerkJ, "ApplicationClerk");
            }
          }
        break;
      }

      if (i == NUM_APPCLERKS-1) {
        for(j = 0; j < NUM_APPCLERKS; j++) {
          currClerkJ = GetMV(appClerks, j);
          state = GetMV(currClerkJ, ACState);
          lineSizeJ = GetMV(currClerkJ, ACCount);

          if (lineSizeJ > 0 && state == ONBREAK) {
            managerWakeup(currClerkJ, "ApplicationClerk");
          }
        }
      }
    }

    for(i = 0; i < NUM_PICCLERKS; i++) {
      currClerkI = GetMV(picClerks, i);
      lineSizeI = GetMV(currClerkI, ACCount)
      if(lineSizeI) > 2) {
          for(j = 0; j < NUM_PICCLERKS; j++) {
            currClerkJ = GetMV(picClerks, j);
            state = GetMV(currClerkJ, ACState);

            if(state == ONBREAK) {
              managerWakeupPicClerk(currClerkJ, "PictureClerk");
            }
          }
          break;
        }

      if (i == NUM_PICCLERKS-1) {
        for(j = 0; j < NUM_PICCLERKS; j++) {
          currClerkJ = GetMV(appClerks, j);
          state = GetMV(currClerkJ, ACState);
          lineSizeJ = GetMV(currClerkJ, ACCount);
          if (lineSizeJ > 0 && state == ONBREAK) {
              managerWakeupPicClerk(currClerkJ, "PictureClerk");
          }
        }
      }
    }
  }
}

int main() {
  setup();
  startManager();
}
