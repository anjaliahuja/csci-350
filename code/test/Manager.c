#include "syscall.h"
#include "Setup.h"
#include <string>

void managerWakeup(int clerk, std::string clerkType) {
  int lock, cv;
  lock = GetMV(clerk, Lock);
  cv = GetMV(clerk, CV);

  Acquire(lock);
  Signal(lock, cv);
  Printf("Manager has woken up " + clerkType + " %d \n", sizeof("Manager has woken up " + clerkType + " %d \n"), (clerk->id));
  Wait(lock, cv);
  Release(lock);
}

void startManager() {
  int i, j, currClerkI, currClerkJ, lineSizeI, lineSizeJ, state, total;


  while(true) {
    for(i = 0; i < NUM_APPCLERKS; i++) {
      /*If the clerk is on break, aka their state is 2 and their line has more than 3 people
      Wake up the thread*/
      currClerkI = GetMV(appClerks, i);
      lineSizeI = GetMV(currClerkI, Count)
      if(lineSizeI > 2) {
          for(j = 0; j < NUM_APPCLERKS; j++) {
            currClerkJ = GetMV(appClerks, j);
            state = GetMV(currClerkJ, State);

            if(state == ONBREAK) {
              managerWakeup(currClerkJ, "ApplicationClerk");
            }
          }
        break;
      }

      if (i == NUM_APPCLERKS-1) {
        for(j = 0; j < NUM_APPCLERKS; j++) {
          currClerkJ = GetMV(appClerks, j);
          state = GetMV(currClerkJ, State);
          lineSizeJ = GetMV(currClerkJ, Count);

          if (lineSizeJ > 0 && state == ONBREAK) {
            managerWakeup(currClerkJ, "ApplicationClerk");
          }
        }
      }
    }

    for(i = 0; i < NUM_PICCLERKS; i++) {
      currClerkI = GetMV(picClerks, i);
      lineSizeI = GetMV(currClerkI, Count)
      if(lineSizeI) > 2) {
          for(j = 0; j < NUM_PICCLERKS; j++) {
            currClerkJ = GetMV(picClerks, j);
            state = GetMV(currClerkJ, State);

            if(state == ONBREAK) {
              managerWakeup(currClerkJ, "PictureClerk");
            }
          }
          break;
        }

      if (i == NUM_PICCLERKS-1) {
        for(j = 0; j < NUM_PICCLERKS; j++) {
          currClerkJ = GetMV(appClerks, j);
          state = GetMV(currClerkJ, State);
          lineSizeJ = GetMV(currClerkJ, Count);
          if (lineSizeJ > 0 && state == ONBREAK) {
              managerWakeup(currClerkJ, "PictureClerk");
          }
        }
      }
    }

    for(i = 0; i < NUM_PASSPORTCLERKS; i++) {
      currClerkI = GetMV(passportClerks, i);
      lineSizeI = GetMV(currClerkI, Count)
      if(lineSizeI > 2) {
          for(j = 0; j < NUM_PASSPORTCLERKS; j++) {
            currClerkJ = GetMV(passportClerks, j);
            state = GetMV(currClerkJ, State);
            if(state == ONBREAK) {
              managerWakeup(currClerkJ, "PassportClerk");
            }
          }
          break;
        }

      if (i == NUM_PASSPORTCLERKS-1) {
        for(j = 0; j < NUM_PASSPORTCLERKS; j++) {
          currClerkJ = GetMV(passportClerks, j);
          state = GetMV(currClerkJ, State);
          lineSizeJ = GetMV(currClerkJ, Count);
          if (lineSizeJ > 0 && state == ONBREAK) {
            managerWakeup(currClerkJ, "PassportClerk");
          }
        }
      }
    }

    for(i = 0; i < NUM_CASHIERS; i++) {
      currClerkI = GetMV(cashiers, i);
      lineSizeI = GetMV(currClerkI, Count)
      if(lineSizeI > 2) {
        for(j = 0; j < NUM_CASHIERS; j++) {
          currClerkJ = GetMV(cashiers, j);
          state = GetMV(currClerkJ, State);
          if(state == ONBREAK) {
            managerWakeup(currClerkJ, "Cashier");
          }
        }
        break;
      }

      if (i == NUM_CASHIERS-1) {
        for(j = 0; j < NUM_CASHIERS; j++) {
          currClerkJ = GetMV(cashiers, j);
          state = GetMV(currClerkJ, State);
          lineSizeJ = GetMV(currClerkJ, Count);
          if (lineSizeJ > 0 && state == 2) {
            managerWakeup(currClerkJ, "Cashier");
          }
        }
      }
    }
  
    for(i = 0; i < 500; i++) {
      Yield();
    }
        /* no more customers */
    if (numCustomers == 0) {
      for(j = 0; j < NUM_APPCLERKS; j++) {
        currClerkJ = GetMV(appClerks, j);
        state = GetMV(currClerkJ, State);
        if(state == ONBREAK) {
          managerWakeup(currClerkJ, "ApplicationClerk");
        }
      }
      for(j = 0; j < NUM_PICCLERKS; j++) {
        currClerkJ = GetMV(picClerks, j);
        state = GetMV(currClerkJ, State);
        if (state == ONBREAK) {
          managerWakeup(currClerkJ, "PictureClerk");
        }
      }
      for(j = 0; j < NUM_PASSPORTCLERKS; j++) {
        currClerkJ = GetMV(passportClerks, j);
        state = GetMV(currClerkJ, State);
        if (state == ONBREAK) {
          managerWakeup(currClerkJ, "PassportClerk");
        }
      }
      for(j = 0; j < NUM_CASHIERS; j++) {
        currClerkJ = GetMV(cashiers, j);
        state = GetMV(currClerkJ, State);
        if (state == ONBREAK) {
          managerWakeup(currClerkJ, "Cashier");
        }
      }

      total = GetMV(AppClerkBribeMoney, 0) + GetMV(PicClerkBribeMoney,0) + GetMV(PassportClerkBribeMoney,0) + GetMV(CashierMoney,0);

      Printf("Manager has has counted a total of %d for Application Clerks \n", sizeof("Manager has has counted a total of %d for Application Clerks \n"), GetMV(AppClerkBribeMoney, 0));
      Printf("Manager has has counted a total of %d for Picture Clerks \n", sizeof("Manager has has counted a total of %d for Picture Clerks \n"), GetMV(PicClerkBribeMoney,0));
      Printf("Manager has has counted a total of %d for Passport Clerks \n", sizeof("Manager has has counted a total of %d for Passport Clerks \n"), GetMV(PassportClerkBribeMoney,0));
      Printf("Manager has has counted a total of %d for Cashiers \n", sizeof("Manager has has counted a total of %d for Cashiers\n"), GetMV(CashierMoney,0));
      Printf("Manager has has counted a total of %d for the Passport Office \n", sizeof("Manager has has counted a total of %d for the Passport Office \n"), total);
      
      break;
    }
    total = GetMV(AppClerkBribeMoney, 0) + GetMV(PicClerkBribeMoney,0) + GetMV(PassportClerkBribeMoney,0) + GetMV(CashierMoney,0);

    Printf("Manager has has counted a total of %d for Application Clerks \n", sizeof("Manager has has counted a total of %d for Application Clerks \n"), GetMV(AppClerkBribeMoney, 0));
    Printf("Manager has has counted a total of %d for Picture Clerks \n", sizeof("Manager has has counted a total of %d for Picture Clerks \n"), GetMV(PicClerkBribeMoney,0));
    Printf("Manager has has counted a total of %d for Passport Clerks \n", sizeof("Manager has has counted a total of %d for Passport Clerks \n"), GetMV(PassportClerkBribeMoney,0));
    Printf("Manager has has counted a total of %d for Cashiers \n", sizeof("Manager has has counted a total of %d for Cashiers\n"), GetMV(CashierMoney,0));
    Printf("Manager has has counted a total of %d for the Passport Office \n", sizeof("Manager has has counted a total of %d for the Passport Office \n"), total);
  }
  Exit(0);
}

int main() {
  setup();
  startManager();
}
