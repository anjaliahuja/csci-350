#include "syscall.h"
#include "Setup.h"

void startPassportClerk() {
  int i, id, random, me, customer;
  Acquire(DataLock);
  id = numActivePassportClerks;
  SetMV(numActivePassportClerks, 0, GetMV(numActivePassportClerks, 0)+1);
  Release(DataLock);


  while(true) {
    if(GetMV(numCustomers, 0) == 0) break;
    Acquire(PassportClerkLineLock);

    me = GetMV(passportClerks, id);

    if (GetMV(me, BribeLineCount) != 0) {
      Signal(PassportClerkLineLock, GetMV(me, BribeLineCV));
      /* wait so that CurrentCust can be set by Customer */
      Wait(PassportClerkLineLock, GetMV(me, BribeLineCV));
      SetMV(PassportClerkBribeMoney, 0, GetMV(PassportClerkBribeMoney,0)+500);
      SetMV(me, State, BUSY);Printf("PassportClerk %d has received $500 from Customer %d\n",
        sizeof("PassportClerk %d has received $500 from Customer %d\n"),
        id*1000+GetMV(me, CurrentCust));
    } else if (GetMV(me, LineCount) != 0) {
      Signal(PassportClerkLineLock, GetMV(me, LineCV));
      /* wait so that CurrentCust can be set by Customer */
      Wait(PassportClerkLineLock, GetMV(me, LineCV));
      SetMV(me, State, BUSY);
      Printf("Passport clerk %d has signalled customer to come to their counter\n", sizeof("Passport clerk %d has signalled customer to come to their counter."), id);
    } else {
      Acquire(GetMV(me, Lock));
      Release(PassportClerkLineLock);
      SetMV(me, State, ONBREAK);
      Printf("Passport clerk %d is going on break\n", sizeof("Passport clerk %d is going on break\n"), id);
      Wait(GetMV(me, Lock), GetMV(me, CV));
      Printf("Passport clerk %d is coming off break\n", sizeof("Passport clerk %d is coming off break\n"), id);
      Signal(GetMV(me, Lock), GetMV(me, CV));
      SetMV(me, State, AVAIL);

      Release(GetMV(me, Lock));
      continue;
    }
    Acquire(GetMV(me, Lock));
    Release(PassportClerkLineLock);

    Wait(GetMV(me, Lock), GetMV(me, CV));

    Printf("Passport clerk %d has received SSN %d from Customer %d\n", sizeof("Passport clerk %d has received SSN from Customer %d\n"), 
      id*1000000+GetMV(me, CurrentCust)*1000+GetMV(me, CurrentCust));

    /* 5% chance that passport clerk makes a mistake.*/
    random = Rand(4, 0);
    if (random == 0) {
      Printf("Passport clerk %d has determined that Customer %d does not have both their application and picture completed\n", sizeof("Passport clerk %d has determined that Customer %d does not have both their application and picture completed\n"), 
        id*1000+GetMV(me, CurrentCust));
      customer = GetMV(customers, GetMV(me, CurrentCust));
      SetMV(customer, SendToBack, true);
      Signal(GetMV(me, Lock), GetMV(me, CV));
    }
    else {
      Printf("Passport clerk %d has determined that Customer %d has both their application and picture completed\n", 
        sizeof("Passport clerk %d has determined that Customer %d has both their application and picture completed\n"), 
        id*1000+GetMV(me, CurrentCust));
      Signal(GetMV(me, Lock), GetMV(me, CV));
      Release(GetMV(me, Lock));
      for(i =20; i<100; ++i){
          Yield();
      }
      Acquire(GetMV(me, Lock));
      Signal(GetMV(me, Lock), GetMV(me, CV));
      Printf("Passport clerk %d has recorded Customer %d passport documentation\n", sizeof("Passport clerk %d has recorded Customer %d passport documentation\n"), 
        id*1000+GetMV(me, CurrentCust));
      
    }

    Wait(GetMV(me, Lock), GetMV(me, CV));

    SetMV(me, CurrentCust, -1);
    Release(GetMV(me, Lock));
  }
  Exit(0);
}

int main(){
  setup();
  startPassportClerk();
}
