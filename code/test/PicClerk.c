#include "syscall.h"
#include "Setup.h"

void startPicClerk() {
  int i, id, me;
  Acquire(DataLock);
  id = numActivePicClerks;
  SetMV(numActivePicClerks, 0, GetMV(numActivePicClerks, 0)+1);
  Release(DataLock);

  while(true) {
    if(GetMV(numCustomers, 0) == 0) break;
    Acquire(PicClerkLineLock);

    me = GetMV(picClerks, id);
    if (GetMV(me, BribeLineCount) != 0) {
      Signal(PicClerkLineLock, GetMV(me, BribeLineCV));
      /* wait so that CurrentCust can be set by Customer */
      Wait(PicClerkLineLock, GetMV(me, BribeLineCV));
      SetMV(PicClerkBribeMoney, 0, GetMV(PicClerkBribeMoney,0)+500);
      SetMV(me, State, BUSY);
      Printf("PictureClerk %d has received $500 from Customer %d\n",
        sizeof("PictureClerk %d has received $500 from Customer %d\n"),
        id*1000+GetMV(me, CurrentCust));
    } else if (GetMV(me, LineCount) != 0) {
      Signal(PicClerkLineLock, GetMV(me, LineCV));
      /* wait so that CurrentCust can be set by Customer */
      Wait(PicClerkLineLock, GetMV(me, LineCV));
      SetMV(me, State, BUSY);
      Printf("PictureClerk %d has signalled customer %d to come to their counter\n", sizeof("PictureClerk %d has signalled customer %d to come to their counter\n"), 
        id*1000+GetMV(me, CurrentCust));
    } else {
      Acquire(GetMV(me, Lock));
      Release(PicClerkLineLock);
      SetMV(me, State, ONBREAK);
      Printf("PictureClerk %d is going on break\n", sizeof("PictureClerk %d is going on break"), id);
      Wait(GetMV(me, Lock), GetMV(me, CV));
      Printf("PictureClerk %d coming off break\n", sizeof("PictureClerk %d is coming off break\n"), id);
      Signal(GetMV(me, Lock), GetMV(me, CV));
      SetMV(me, State, AVAIL);

      Release(GetMV(me, Lock));
      continue;
    }

    Acquire(GetMV(me, Lock));
    Release(PicClerkLineLock);

    /* Wait for customer data */
    Wait(GetMV(me, Lock), GetMV(me, CV));
    Printf("PictureClerk %d has received SSN %d from customer %d\n", sizeof("PictureClerk %d has received SSN %d from customer %d\n"), 
      id*1000000+GetMV(me, CurrentCust)*1000+GetMV(me, CurrentCust));
   
    /* Do my job, customer now waiting */
    Signal(GetMV(me, Lock), GetMV(me, CV));
    Printf("PictureClerk %d has taken picture of customer %d\n", sizeof("PictureClerk %d has taken picture of customer %d\n"), 
      id*1000+ GetMV(me, CurrentCust));
   

    /* waiting for approval */
    Wait(GetMV(me, Lock), GetMV(me, CV));

    if (!GetMV(me, LikePicture)) {
      Printf("PictureClerk %d has been told that customer %d does not like their picture.\n", sizeof("PictureClerk %d has been told that customer %d does not like their picture.\n"), 
        id*1000+ GetMV(me, CurrentCust));
      Signal(GetMV(me, Lock), GetMV(me, CV));
    }
    else {
      Printf("PictureClerk %d has been told that customer %d does like their picture.\n", sizeof("PictureClerk %d has been told that customer %d does like their picture.\n"), 
        id*1000+GetMV(me, CurrentCust));

      Release(GetMV(me, Lock));
      for(i =20; i<100; ++i){
          Yield();
      }
      Acquire(GetMV(me, Lock));
      Signal(GetMV(me, Lock), GetMV(me, CV));
    }
    Wait(GetMV(me, Lock), GetMV(me, CV));

    SetMV(me, CurrentCust, -1);
    Release(GetMV(me, Lock));
  }
  Exit(0);
}

int main() {
  setup();
  startPicClerk();
}
