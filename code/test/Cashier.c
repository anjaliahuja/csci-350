#include "syscall.h"
#include "Setup.h"

void startCashier() {
  int i, id, random, me, customer;
  Acquire(DataLock);
  id = GetMV(numActiveCashiers, 0);
  SetMV(numActiveCashiers, 0, GetMV(numActiveCashiers, 0)+1);
  Release(DataLock);

  while(true) {
    if(GetMV(numCustomers, 0) == 0) break;
    Acquire(CashierLineLock);
    
    me = GetMV(cashiers, id);
    if (GetMV(me, BribeLineCount) != 0) {
      Signal(CashierLineLock, GetMV(me, BribeLineCV));
      /* wait so that CurrentCust can be set by Customer */
      Wait(CashierLineLock, GetMV(me, BribeLineCV));
      SetMV(CashierMoney, 0, GetMV(CashierMoney,0)+500);
      SetMV(me, State, BUSY);
      Printf("Cashier %d has received $500 from Customer %d\n",
        sizeof("Cashier %d has received $500 from Customer %d\n"),
        id*1000+GetMV(me, CurrentCust));
    } else if (GetMV(me, LineCount) != 0) {
      Signal(CashierLineLock, GetMV(me, LineCV));
      /* wait so that CurrentCust can be set by Customer */
      Wait(CashierLineLock, GetMV(me, LineCV));
      SetMV(me, State, BUSY);
      Printf("Cashier %d has signalled customer to come to their counter\n", sizeof("Cashier %d has signalled customer to come to their counter."), id);
    } else {
      Acquire(GetMV(me, Lock));
      Release(CashierLineLock);
      SetMV(me, State, ONBREAK);
      Printf("Cashier_ %d is going on break\n", sizeof("Cashier_ %d is going on break\n"), id);

      Wait(GetMV(me, Lock), GetMV(me, CV));
      Printf("Cashier_ %d is coming off break\n", sizeof("Cashier_ %d is coming off break\n"), id);
      Signal(GetMV(me, Lock), GetMV(me, CV));
      SetMV(me, State, AVAIL);

      Release(GetMV(me, Lock));
      continue;
    }
    Acquire(GetMV(me, Lock));
    Release(CashierLineLock);

    Wait(GetMV(me, Lock), GetMV(me, CV));

    Printf("Cashier_%d has received SSN from Customer_%d \n", sizeof("Cashier_%d has received SSN from Customer_%d \n"), 
      (id*1000+GetMV(me, CurrentCust)));

    /* 5% chance that passport clerk makes a mistake.*/
    random = Rand(4, 0);
    if (random == 0) {
      Printf("Cashier_%d has received $100 from Customer_%d \n", sizeof("Cashier_%d has received $100 from Customer_%d "), 
        (id*1000+GetMV(me, CurrentCust)));
      Write(" before certification. They are to go to the back of the line \n", sizeof(" before certification. They are to go to the back of the line \n"), ConsoleOutput);

      customer = GetMV(customers, GetMV(me, CurrentCust));
      SetMV(customer, SendToBack, true);
      Signal(GetMV(me, Lock), GetMV(me, CV));
    }
    else {
      Signal(GetMV(me, Lock), GetMV(me, CV));

      Printf("Cashier_%d has verified that Customer_%d \n", sizeof("Cashier_%d has has verified that Customer_%d \n"), 
        (id*1000+GetMV(me, CurrentCust)));
      Write(" has been certified by a PassportClerk \n", sizeof(" has been certified by a PassportClerk \n"), ConsoleOutput);

      Wait(GetMV(me, Lock), GetMV(me, CV));
      SetMV(CashierMoney, 0, GetMV(CashierMoney,0)+100);

      Printf("Cashier_%d has received the $100 from Customer_%d \n", sizeof("Cashier_%d has received the $100 from Customer_%d \n"), 
        (id*1000+GetMV(me, CurrentCust)));
      Write(" after certification \n", sizeof(" after certification \n"), ConsoleOutput);

      Release(GetMV(me, Lock));
      for(i =20; i<100; ++i){
          Yield();
      }
      Acquire(GetMV(me, Lock));
      Printf("Cashier_%d has provided Customer_%d their completed passport \n", sizeof("Cashier_%d has provided Customer_%d their completed passport \n"), 
        (id*1000+GetMV(me, CurrentCust)));
      Signal(GetMV(me, Lock), GetMV(me, CV));
      Wait(GetMV(me, Lock), GetMV(me, CV));

      Printf("Cashier_%d has recoreded that Customer_%d \n", sizeof("Cashier_%d has recoreded that Customer_%d "), 
        (id*1000+GetMV(me, CurrentCust)));
      Write(" has been given their completed passport \n", sizeof(" has been given their completed passport \n"), ConsoleOutput);

      Signal(GetMV(me, Lock), GetMV(me, CV));
    }

    Wait(GetMV(me, Lock), GetMV(me, CV));

    SetMV(me, CurrentCust, NULL);
    Release(GetMV(me, Lock));
  }
  Exit(0);
}

void main() {
  setup();
  startCashier();
}
