#include "syscall.h"
#include "Setup.h"
#include <string>

void startAppClerk() {
  int i, id, me;
  Acquire(DataLock);
  id = numActiveAppClerks;
  // numActiveAppClerks++;
  value = GetMV(numActiveAppClerks, 0);
  SetMV(numActiveAppClerks, 0, value+1);
  Release(DataLock);

  while(true) {
    if(GetMV(numCustomers, 0) == 0) break;
    Acquire(AppClerkLineLock);

    me = GetMV(appClerks, id);
    if (GetMV(me, BribeLineCount) != 0) {
      Signal(AppClerkLineLock, GetMV(me, BribeLineCV));
      /* wait so that CurrentCust can be set by Customer */
      Wait(AppClerkLineLock, GetMV(me, BribeLineCV));
      SetMV(AppClerkBribeMoney, 0, GetMV(AppClerkBribeMoney,0)+500);
      SetMV(me, State, BUSY);
      Printf("ApplicationClerk %d has received $500 from Customer %d\n",
        sizeof("ApplicationClerk %d has received $500 from Customer %d\n"),
        id*1000+GetMV(me, CurrentCust));
    } else if (GetMV(me, LineCount) != 0) {
      Signal(AppClerkLineLock, GetMV(me, LineCV));
      /* wait so that CurrentCust can be set by Customer */
      Wait(AppClerkLineLock, GetMV(me, LineCV));
      SetMV(me, State, BUSY);
      Printf("ApplicationClerk %d has signalled a Customer to come to their counter\n",
        sizeof("ApplicationClerk %d has signalled a Customer to come to their counter\n"),
        id);
    } else {
      Acquire(GetMV(me, Lock));
      Release(AppClerkLineLock);
      SetMV(me, State, ONBREAK);
      Printf("ApplicationClerk %d is going on break\n",
        sizeof("ApplicationClerk %d is going on break\n"),
        id);
      Wait(GetMV(me, Lock), GetMV(me, CV));
      Printf("ApplicationClerk %d is coming off break\n",
        sizeof("ApplicationClerk %d is coming off break\n"),
        id);
      Signal(GetMV(me, Lock), GetMV(me, CV));
      SetMV(me, State, AVAIL);

      Release(GetMV(me, Lock));
      continue;
    }
    Acquire(GetMV(me, Lock));
    Release(AppClerkLineLock);

    Wait(GetMV(me, Lock), GetMV(me, CV));
    Printf("ApplicationClerk %d has received SSN %d from Customer %d\n",
        sizeof("ApplicationClerk %d has received SSN %d from Customer %d\n"),
        id*1000000+GetMV(me, CurrentCust)*1000+GetMV(me, CurrentCust));

    Release(GetMV(me, Lock));
    for(i =20; i<100; ++i){
      Yield();
    }
    Acquire(GetMV(me, Lock));
    Signal(GetMV(me, Lock), GetMV(me, CV));
    Printf("ApplicationClerk %d has recorded a completed application for Customer %d\n",
        sizeof("ApplicationClerk %d has recorded a completed application for Customer %d\n"),
        id*1000+GetMV(me, CurrentCust));

    Wait(GetMV(me, Lock), GetMV(me, CV));

    SetMV(me, CurrentCust, -1);
    Release(GetMV(me, Lock));
  }
  Exit(0);
}

int main(){
  setup();
  startAppClerk();
}
