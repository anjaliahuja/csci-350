#include "syscall.h"
#include "Setup.h"

int findLine(char type, int customer) {
  /* variable declarations */
  int my_line = -1;
  int line_size = 9999;
  int i, currClerkI, lineSizeI;
  int me, myClerk;
  int random = 0;

  /* pass in a char to decide which type of line to find
    'a' is AppClerk, 'p' is PicClerk, 's' is PassportClerk, 'c' is cashier
  */
  if(type == 'a') {
    /* Picking a line */
    Acquire(AppClerkLineLock);
    for(i = 0; i < NUM_APPCLERKS; i++) {
      currClerkI = GetMV(appClerks, i);
      lineSizeI = GetMV(currClerkI, LineCount);
      if(lineSizeI < line_size) {
        line_size = lineSizeI;
        my_line = i;
      }
    }

    /* Bribe */
    random = Rand(9, 0);

    me = GetMV(customers, customer);
    myClerk = GetMV(appClerks, my_line);

    if (GetMV(me, Money) >= 600 && GetMV(myClerk, State) == BUSY && random < 3) {
      SetMV(myClerk, BribeLineCount, GetMV(myClerk, BribeLineCount)+1);
      Printf("Customer %d has gotten in bribe line for ApplicationClerk %d\n", 
        sizeof("Customer %d has gotten in bribe line for ApplicationClerk %d\n"),
        GetMV(me, SSN)*1000+GetMV(myClerk, ID));
      Wait(AppClerkLineLock, GetMV(myClerk, BribeLineCV));
      SetMV(myClerk, BribeLineCount, GetMV(myClerk, BribeLineCount)-1);
      SetMV(myClerk, CurrentCust, customer);
      SetMV(me, Money, GetMV(me, Money)-500);
      Signal(AppClerkLineLock, GetMV(myClerk, BribeLineCV));
    } else {
      if (GetMV(myClerk, State) == BUSY || GetMV(myClerk, State) == ONBREAK) {
        SetMV(myClerk, LineCount, GetMV(myClerk, LineCount)+1);
        Printf("Customer %d has gotten in regular line for ApplicationClerk %d\n", 
          sizeof("Customer %d has gotten in regular line for ApplicationClerk %d\n"), 
          GetMV(me, SSN)*1000+GetMV(myClerk, ID));
        Wait(AppClerkLineLock, GetMV(myClerk, LineCV));
        SetMV(myClerk, LineCount, GetMV(myClerk, LineCount)-1);
        SetMV(myClerk, CurrentCust, customer);
        Signal(AppClerkLineLock, GetMV(myClerk, LineCV));
      } else {
        SetMV(myClerk, CurrentCust, customer);
      }
    }

    SetMV(myClerk, State, BUSY);
    Release(AppClerkLineLock);

    return my_line;
  } else if (type == 'p') {
    /* Picking a line */
    Acquire(PicClerkLineLock);
    for(i = 0; i < NUM_PICCLERKS; i++) {
      currClerkI = GetMV(picClerks, i);
      lineSizeI = GetMV(currClerkI, LineCount);
      if(lineSizeI < line_size) {
        line_size = lineSizeI;
        my_line = i;
      }
    }

    /* Bribe */
    random = Rand(9, 0);
    
    me = GetMV(customers, customer);
    myClerk = GetMV(picClerks, my_line);

    if (GetMV(me, Money) >= 600 && GetMV(myClerk, State) == BUSY && random < 3) {
      SetMV(myClerk, BribeLineCount, GetMV(myClerk, BribeLineCount)+1);      
      Printf("Customer %d has gotten in bribe line for PictureClerk %d\n", 
        sizeof("Customer %d has gotten in bribe line for PictureClerk %d\n"),
        GetMV(me, SSN)*1000+GetMV(myClerk, ID));
      Wait(PicClerkLineLock, GetMV(myClerk, BribeLineCV));
      SetMV(myClerk, BribeLineCount, GetMV(myClerk, BribeLineCount)-1);
      SetMV(myClerk, CurrentCust, customer);
      SetMV(me, Money, GetMV(me, Money)-500);
      Signal(PicClerkLineLock, GetMV(myClerk, BribeLineCV));
    } else {
      if (GetMV(myClerk, State) == BUSY || GetMV(myClerk, State) == ONBREAK) {
        SetMV(myClerk, LineCount, GetMV(myClerk, LineCount)+1);
        Printf("Customer %d has gotten in regular line for PictureClerk %d\n", 
          sizeof("Customer %d has gotten in regular line for PictureClerk %d\n"), 
          GetMV(me, SSN)*1000+GetMV(myClerk, ID));
        Wait(PicClerkLineLock, GetMV(myClerk, LineCV));
        SetMV(myClerk, LineCount, GetMV(myClerk, LineCount)-1);
        SetMV(myClerk, CurrentCust, customer);
        Signal(PicClerkLineLock, GetMV(myClerk, LineCV));
      } else {
        SetMV(myClerk, CurrentCust, customer);
      }
    }

    SetMV(myClerk, State, BUSY);
    Release(PicClerkLineLock);

    return my_line;
  } else if (type == 's') {
    /* Picking a line */
    Acquire(PassportClerkLineLock);
    for(i = 0; i < NUM_PASSPORTCLERKS; i++) {
      currClerkI = GetMV(passportClerks, i);
      lineSizeI = GetMV(currClerkI, LineCount);
      if(lineSizeI < line_size) {
        line_size = lineSizeI;
        my_line = i;
      }
    }

    /* Bribe */
    random = Rand(9, 0);

    me = GetMV(customers, customer);
    myClerk = GetMV(passportClerks, my_line);

    if (GetMV(me, Money) >= 600 && GetMV(myClerk, State) == BUSY && random < 3) {
      SetMV(myClerk, BribeLineCount, GetMV(myClerk, BribeLineCount)+1);
      Printf("Customer %d has gotten in bribe line for PassportClerk %d\n", 
        sizeof("Customer %d has gotten in bribe line for PassportClerk %d\n"),
        GetMV(me, SSN)*1000+GetMV(myClerk, ID));
      Wait(PassportClerkLineLock, GetMV(myClerk, BribeLineCV));
      SetMV(myClerk, BribeLineCount, GetMV(myClerk, BribeLineCount)-1);
      SetMV(myClerk, CurrentCust, customer);
      SetMV(me, Money, GetMV(me, Money)-500);
      Signal(PassportClerkLineLock, GetMV(myClerk, BribeLineCV));
    } else {
      if (GetMV(myClerk, State) == BUSY || GetMV(myClerk, State) == ONBREAK) {
        SetMV(myClerk, LineCount, GetMV(myClerk, LineCount)+1);
        Printf("Customer %d has gotten in regular line for PassportClerk %d\n", 
          sizeof("Customer %d has gotten in regular line for PassportClerk %d\n"), 
          GetMV(me, SSN)*1000+GetMV(myClerk, ID));
        Wait(PassportClerkLineLock, GetMV(myClerk, LineCV));
        SetMV(myClerk, LineCount, GetMV(myClerk, LineCount)-1);
        SetMV(myClerk, CurrentCust, customer);
        Signal(PassportClerkLineLock, GetMV(myClerk, LineCV));
      } else {
        SetMV(myClerk, CurrentCust, customer);
      }
    }

    SetMV(myClerk, State, BUSY);
    Release(PassportClerkLineLock);

    return my_line;
  } else if (type == 'c') {
    /* Picking a line */
    Acquire(CashierLineLock);
    for(i = 0; i < NUM_CASHIERS; i++) {
      currClerkI = GetMV(cashiers, i);
      lineSizeI = GetMV(currClerkI, LineCount);
      if(lineSizeI < line_size) {
        line_size = lineSizeI;
        my_line = i;
      }
    }

    /* Bribe */
    random = Rand(9, 0);

    me = GetMV(customers, customer);
    myClerk = GetMV(cashiers, my_line);

    if (GetMV(me, Money) >= 600 && GetMV(myClerk, State) == BUSY && random < 3) {
      SetMV(myClerk, BribeLineCount, GetMV(myClerk, BribeLineCount)+1);
      Printf("Customer %d has gotten in bribe line for Cashier %d\n", 
        sizeof("Customer %d has gotten in bribe line for Cashier %d\n"),
        GetMV(me, SSN)*1000+GetMV(myClerk, ID));
      Wait(CashierLineLock, GetMV(myClerk, BribeLineCV));
      SetMV(myClerk, BribeLineCount, GetMV(myClerk, BribeLineCount)-1);
      SetMV(myClerk, CurrentCust, customer);
      SetMV(me, Money, GetMV(me, Money)-500);
      Signal(CashierLineLock, GetMV(myClerk, BribeLineCV));
    } else {
      if (GetMV(myClerk, State) == BUSY || GetMV(myClerk, State) == ONBREAK) {
        SetMV(myClerk, LineCount, GetMV(myClerk, LineCount)+1);
        Printf("Customer %d has gotten in regular line for Cashier %d\n", 
          sizeof("Customer %d has gotten in regular line for Cashier %d\n"), 
         GetMV(me, SSN)*1000+GetMV(myClerk, ID));
        Wait(CashierLineLock, GetMV(myClerk, LineCV));
        SetMV(myClerk, LineCount, GetMV(myClerk, LineCount)-1);
        SetMV(myClerk, CurrentCust, customer);
        Signal(CashierLineLock, GetMV(myClerk, LineCV));
      } else {
        SetMV(myClerk, CurrentCust, customer);
      }
    }

    SetMV(myClerk, State, BUSY);
    Release(CashierLineLock);

    return my_line;
  }
}

void getAppFiled(int my_line, int customer) {
  int myClerk, me;

  myClerk = GetMV(appClerks, my_line);
  me = GetMV(customers, customer);

  Acquire(GetMV(myClerk, Lock));
  Signal(GetMV(myClerk, Lock), GetMV(myClerk, CV));
  Printf("Customer %d has given SSN %d to ApplicationClerk %d\n",
    sizeof("Customer %d has given SSN %d to ApplicationClerk %d\n"),
    customer*1000000+customer*1000+GetMV(appClerks, ID));
  Wait(GetMV(myClerk, Lock), GetMV(myClerk, CV));

  Signal(GetMV(myClerk, Lock), GetMV(myClerk, CV));
  Release(GetMV(myClerk, Lock));
}

void getPicTaken(int my_line, int customer) {
  int dislikePicture = 0, i;
  int myClerk, me;

  myClerk = GetMV(picClerks, my_line);
  me = GetMV(customers, customer);

  Acquire(GetMV(myClerk, Lock));

  Signal(GetMV(myClerk, Lock), GetMV(myClerk, CV));
  Printf("Customer %d has given SSN %d to PictureClerk %d\n", sizeof("Customer %d has given SSN %d to PictureClerk %d\n"), customer*1000000+customer*1000+GetMV(myClerk, ID));

  /* Waits for PictureClerk to take picture */
  Wait(GetMV(myClerk, Lock), GetMV(myClerk, CV));
  dislikePicture = Rand(100, 1); /*chooses percentage between 1-99*/
  if (dislikePicture > 50)  {
    Printf("Customer %d does not like their picture from PicClerk %d\n", sizeof("Customer %d does not like their picture from PicClerk %d\n"), customer*1000+GetMV(myClerk, ID));

      SetMV(myClerk, LikePicture, false);
      Signal(GetMV(myClerk, Lock), GetMV(myClerk, CV));
      /* Has to get back in a line.
         Wait to make sure that clerk acknowledges & then go back in line
      */
      Wait(GetMV(myClerk, Lock), GetMV(myClerk, CV));

      /*Signal clerk that I'm leaving*/
      Signal(GetMV(myClerk, Lock), GetMV(myClerk, CV));
      
      /* Implemented so that the if the clerk they were at can become available again
         Not necessarily punishment
      */
      Release(GetMV(myClerk, Lock));
      for(i =100; i<1000; ++i){
          Yield();
      }

      getPicTaken(findLine('p', customer), customer);
      return;
  }

  SetMV(myClerk, LikePicture, true);

  Printf("Customer %d likes their picture from PicClerk %d\n", sizeof("Customer %d likes their picture from PicClerk %d\n"), customer*1000+GetMV(myClerk, ID));

  /* 
  Signal clerk and Wait to make sure that clerk acknowledges.
  */
  Signal(GetMV(myClerk, Lock), GetMV(myClerk, CV));
  Wait(GetMV(myClerk, Lock), GetMV(myClerk, CV));

  /* Signal clerk that I'm leaving */
  Signal(GetMV(myClerk, Lock), GetMV(myClerk, CV));
  Release(GetMV(myClerk, Lock));  
}

void getPassport(int my_line, int customer) {
  int i;  
  int myClerk, me;

  myClerk = GetMV(passportClerks, my_line);
  me = GetMV(customers, customer);

  Acquire(GetMV(myClerk, Lock));
  Signal(GetMV(myClerk, Lock), GetMV(myClerk, CV));

  Printf("Customer %d has given SSN %d to PassportClerk %d\n", 
    sizeof("Customer %d has given SSN %d to PassportClerk %d\n"), 
    customer*1000000+customer*1000+GetMV(myClerk, ID));
 
  /* Wait to determine whether they go back in line */
  Wait(GetMV(myClerk, Lock), GetMV(myClerk, CV));

  if(GetMV(me, SendToBack)){
    Printf("Customer %d has gone to PassportClerk %d too soon. They are going to back of line\n", 
      sizeof("Customer %d has gone to PassportClerk %d too soon. They are going to back of line\n"), 
      customer*1000+GetMV(myClerk, ID));
   

    /* Signal clerk that I'm leaving */
    Signal(GetMV(myClerk, Lock), GetMV(myClerk, CV));
    Release(GetMV(myClerk, Lock));
    for(i =100; i<1000; ++i){
        Yield();
    }

    SetMV(me, SendToBack, false);
    /* Assuming back of line does not mean their current line */
    getPassport(findLine('s', customer), customer);
    return;
  } 
  
  /* waits for passport clerk to record passport */
  Wait(GetMV(myClerk, Lock), GetMV(myClerk, CV));    
  
  /* Signal clerk that I'm leaving */
  Signal(GetMV(myClerk, Lock), GetMV(myClerk, CV));
  Release(GetMV(myClerk, Lock));  
}

void payCashier(int my_line, int customer) {
  int i;
  int myClerk, me;

  myClerk = GetMV(cashiers, my_line);
  me = GetMV(customers, customer);

  Acquire(GetMV(myClerk, Lock));
  Signal(GetMV(myClerk, Lock), GetMV(myClerk, CV));

  Printf("Customer_%d has given SSN %d to Cashier_%d \n", 
    sizeof("Customer_%d has given SSN %d to Cashier_%d \n"), 
    (customer*1000000+GetMV(me, SSN)*1000+my_line));

  /* Wait to determine whether they go back in line */
  Wait(GetMV(myClerk, Lock), GetMV(myClerk, CV));

  if(GetMV(me, SendToBack)){
    /* send customer to back of line after yield */
    Printf("Customer_%d has gone to Cashier_%d too soon \n", 
      sizeof("Customer_%d has gone to Cashier_%d too soon \n"), 
      (customer*1000+my_line));
    Write("They are going to the back of the line \n", 
      sizeof("They are going to the back of the line \n"), 
      ConsoleOutput);

    /* Signal cashier that I'm leaving */
    Signal(GetMV(myClerk, Lock), GetMV(myClerk, CV));

    Release(GetMV(myClerk, Lock));
    for(i =100; i<1000; ++i){
        Yield();
    }

    SetMV(me, SendToBack, false);
    payCashier(findLine('c', customer), customer);
    return;
  }
  
  Signal(GetMV(myClerk, Lock), GetMV(myClerk, CV));
  SetMV(me, Money, GetMV(me, Money)-100);
  /*if (test6) MONEY += 100;*/
  
  Printf("Customer_%d has given Cashier_%d $100 \n", sizeof("Customer_%d has given Cashier_%d $100 \n"), (customer*1000+my_line));

  /* waits for cashier to give me passport */
  Wait(GetMV(myClerk, Lock), GetMV(myClerk, CV));

  /* ensure cashier that i've been given my passport */
  Signal(GetMV(myClerk, Lock), GetMV(myClerk, CV));
  Wait(GetMV(myClerk, Lock), GetMV(myClerk, CV));

  /* Signal cashier that I'm leaving */
  Signal(GetMV(myClerk, Lock), GetMV(myClerk, CV));
  Release(GetMV(myClerk, Lock));  
}

void startCustomer(){
  int task, id, value;
  int my_line = -1;

  Acquire(DataLock);
  id = numActiveCustomers;
  value = GetMV(numActiveCustomers, 0);
  SetMV(numActiveCustomers, 0, value+1);
  Release(DataLock);

  /*implementation */
  task = Rand(1, 0);
  if (task == 0){
    my_line = findLine('a', id);
    getAppFiled(my_line, id);
    my_line = findLine('p', id);
    getPicTaken(my_line, id);
  } else if (task == 1) {
    my_line = findLine('p', id);
    getPicTaken(my_line, id);
    my_line = findLine('a', id);
    getAppFiled(my_line, id);
  }

  my_line = findLine('s', id);
  getPassport(my_line, id);
  my_line = findLine('c', id);
  payCashier(my_line, id);

  Printf("Customer %d is leaving the Passport Office\n", 
    sizeof("Customer %d is leaving the Passport Office\n"), 
    id);

  value = GetMV(numCustomers, 0);
  SetMV(numCustomers, 0, value-1);
  Exit(0);
}

int main() {
  setup();
  startCustomer();
}
