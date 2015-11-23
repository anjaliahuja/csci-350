







void startAppClerk() {
  int i, id;
  Acquire(DataLock);
  id = numActiveAppClerks;
  numActiveAppClerks++;
  Release(DataLock);

  while(true) {
    if(numCustomers == 0) break;
    Acquire(AppClerkLineLock);
    /*if (SenatorArrived) {

    } */if (queue_size(&AppClerks[id].bribeLine) != 0) {
      Signal(AppClerkLineLock, AppClerks[id].bribeLineCV);
      AppClerkBribeMoney += 500;
      AppClerks[id].state = 1;
      AppClerks[id].currentCustomer = queue_pop(&AppClerks[id].bribeLine);
      Printf("ApplicationClerk %d has received $500 from Customer %d\n",
        sizeof("ApplicationClerk %d has received $500 from Customer %d\n"),
        id*1000+AppClerks[id].currentCustomer);
    } else if (queue_size(&AppClerks[id].line) != 0) {
      Signal(AppClerkLineLock, AppClerks[id].lineCV);
      AppClerks[id].state = 1;
      AppClerks[id].currentCustomer = queue_pop(&AppClerks[id].line);
      Printf("ApplicationClerk %d has signalled a Customer to come to their counter\n",
        sizeof("ApplicationClerk %d has signalled a Customer to come to their counter\n"),
        id);
    } else {
      Acquire(AppClerks[id].lock);
      Release(AppClerkLineLock);
      AppClerks[id].state = 2;
      Printf("ApplicationClerk %d is going on break\n",
        sizeof("ApplicationClerk %d is going on break\n"),
        id);
      Wait(AppClerks[id].lock, AppClerks[id].cv);
      Printf("ApplicationClerk %d is coming off break\n",
        sizeof("ApplicationClerk %d is coming off break\n"),
        id);
      Signal(AppClerks[id].lock, AppClerks[id].cv);
      AppClerks[id].state = 0;

      Release(AppClerks[id].lock);
      continue;
    }
    Acquire(AppClerks[id].lock);
    Release(AppClerkLineLock);

    Wait(AppClerks[id].lock, AppClerks[id].cv);
    Printf("ApplicationClerk %d has received SSN %d from Customer %d\n",
        sizeof("ApplicationClerk %d has received SSN %d from Customer %d\n"),
        id*1000000+AppClerks[id].currentCustomer*1000+AppClerks[id].currentCustomer);

    Release(AppClerks[id].lock);
    for(i =20; i<100; ++i){
      Yield();
    }
    Acquire(AppClerks[id].lock);
    Signal(AppClerks[id].lock, AppClerks[id].cv);
    Printf("ApplicationClerk %d has recorded a completed application for Customer %d\n",
        sizeof("ApplicationClerk %d has recorded a completed application for Customer %d\n"),
        id*1000+AppClerks[id].currentCustomer);

    Wait(AppClerks[id].lock, AppClerks[id].cv);
    if (SenatorArrived) {
      Acquire(SenatorLock);
      Wait(SenatorLock, AppClerks[id].senatorCV);
      Release(SenatorLock);
    }

    AppClerks[id].currentCustomer = -1;
    Release(AppClerks[id].lock);
  }
  Exit(0);
}

void startPicClerk() {
  int i, id;
  Acquire(DataLock);
  id = numActivePicClerks;
  numActivePicClerks++;
  Release(DataLock);

  while(true) {
    if (numCustomers == 0) break;
    Acquire(PicClerkLineLock);

    if (SenatorArrived /*&& !setUpSenator*/) {

    } else if (queue_size(&PicClerks[id].bribeLine) != 0) {
      Signal(PicClerkLineLock, PicClerks[id].bribeLineCV);
      PicClerkBribeMoney += 500;
      PicClerks[id].state = 1;
      PicClerks[id].currentCustomer = queue_pop(&PicClerks[id].bribeLine);
      Printf("PictureClerk %d has received $500 from Customer %d\n",
        sizeof("PictureClerk %d has received $500 from Customer %d\n"),
        id*1000+PicClerks[id].currentCustomer);
    } else if (queue_size(&PicClerks[id].line) != 0) {
      Signal(PicClerkLineLock, PicClerks[id].lineCV);
      PicClerks[id].state = 1;
      PicClerks[id].currentCustomer = queue_pop(&PicClerks[id].line);
      Printf("PictureClerk %d has signalled customer %d to come to their counter\n", sizeof("PictureClerk %d has signalled customer %d to come to their counter\n"), id*1000+(PicClerks[id].currentCustomer));
    } else if (!SenatorArrived) {
      Acquire(PicClerks[id].lock);
      Release(PicClerkLineLock);
      PicClerks[id].state = 2;
      Printf("PictureClerk %d is going on break\n", sizeof("PictureClerk %d is going on break"), id);
      Wait(PicClerks[id].lock, PicClerks[id].cv);
      Printf("PictureClerk %d coming off break\n", sizeof("PictureClerk %d is coming off break\n"), id);
      Signal(PicClerks[id].lock, PicClerks[id].cv);
      Release(PicClerks[id].lock);
      PicClerks[id].state = 0;
      continue;
    }

    Acquire(PicClerks[id].lock);
    Release(PicClerkLineLock);

    /* Wait for customer data */
    Wait(PicClerks[id].lock, PicClerks[id].cv);
    Printf("PictureClerk %d has received SSN %d from customer %d\n", sizeof("PictureClerk %d has received SSN %d from customer %d\n"), id*1000000+(Customers[PicClerks[id].currentCustomer].ssn)*1000+PicClerks[id].currentCustomer);
   
    /* Do my job, customer now waiting */
    Signal(PicClerks[id].lock, PicClerks[id].cv);
    Printf("PictureClerk %d has taken picture of customer %d\n", sizeof("PictureClerk %d has taken picture of customer %d\n"), id*1000+ PicClerks[id].currentCustomer);
   

    /* waiting for approval */
    Wait(PicClerks[id].lock, PicClerks[id].cv);

    if (!PicClerks[id].likePicture) {
      Printf("PictureClerk %d has been told that customer %d does not like their picture.\n", sizeof("PictureClerk %d has been told that customer %d does not like their picture.\n"), id*1000+ PicClerks[id].currentCustomer);
      Signal(PicClerks[id].lock, PicClerks[id].cv);
    }
    else {
      Printf("PictureClerk %d has been told that customer %d does like their picture.\n", sizeof("PictureClerk %d has been told that customer %d does like their picture.\n"), id*1000+ PicClerks[id].currentCustomer);


      Release(PicClerks[id].lock);
      for(i =20; i<100; ++i){
          Yield();
      }
      Acquire(PicClerks[id].lock);
      Signal(PicClerks[id].lock, PicClerks[id].cv);
    }
    Wait(PicClerks[id].lock, PicClerks[id].cv);
    /*if (SenatorArrived && setUpSenator) {
      SenatorLock->Acquire();
      senatorCV->Wait(SenatorLock);
      SenatorLock->Release();
    }*/

    PicClerks[id].currentCustomer = -1;
    Release(PicClerks[id].lock);
  }
  Exit(0);
}

void startPassportClerk() {
  int i, id, random;
  Acquire(DataLock);
  id = numActivePassportClerks;
  numActivePassportClerks++;
  Release(DataLock);

  while(true) {
    if(numCustomers == 0) break;
    Acquire(PassportClerkLineLock);
    if (SenatorArrived) {

    } else if (queue_size(&PassportClerks[id].bribeLine) != 0) {
      Signal(PassportClerkLineLock, PassportClerks[id].bribeLineCV);
      PassportClerkBribeMoney += 500;
      PassportClerks[id].state = 1;
      PassportClerks[id].currentCustomer = queue_pop(&PassportClerks[id].bribeLine);
      Printf("PassportClerk %d has received $500 from Customer %d\n",
        sizeof("PassportClerk %d has received $500 from Customer %d\n"),
        id*1000+PassportClerks[id].currentCustomer);
    } else if (queue_size(&PassportClerks[id].line) != 0) {
      Signal(PassportClerkLineLock, PassportClerks[id].lineCV);
      PassportClerks[id].state = 1;
      PassportClerks[id].currentCustomer = queue_pop(&PassportClerks[id].line);
      Printf("Passport clerk %d has signalled customer to come to their counter\n", sizeof("Passport clerk %d has signalled customer to come to their counter."), id);
    } else {
      Acquire(PassportClerks[id].lock);
      Release(PassportClerkLineLock);
      PassportClerks[id].state = 2;
      Printf("Passport clerk %d is going on break\n", sizeof("Passport clerk %d is going on break\n"), id);
      Wait(PassportClerks[id].lock, PassportClerks[id].cv);
      Printf("Passport clerk %d is coming off break\n", sizeof("Passport clerk %d is coming off break\n"), id);
      Signal(PassportClerks[id].lock, PassportClerks[id].cv);
      PassportClerks[id].state = 0;

      Release(PassportClerks[id].lock);
      continue;
    }
    Acquire(PassportClerks[id].lock);
    Release(PassportClerkLineLock);

    Wait(PassportClerks[id].lock, PassportClerks[id].cv);

    Printf("Passport clerk %d has received SSN from Customer %d\n", sizeof("Passport clerk %d has received SSN from Customer %d\n"), 
      id*1000+(Customers[PassportClerks[id].currentCustomer].ssn));

    /* 5% chance that passport clerk makes a mistake.*/
    random = Rand(4, 0);
    if (random == 0) {
      Printf("Passport clerk %d has determined that Customer %d does not have both their application and picture completed\n", sizeof("Passport clerk %d has determined that Customer %d does not have both their application and picture completed\n"), id*1000+(Customers[PassportClerks[id].currentCustomer].ssn));
      Customers[PassportClerks[id].currentCustomer].sendToBackOfLine = true;
      Signal(PassportClerks[id].lock, PassportClerks[id].cv);
    }
    else {
      Printf("Passport clerk %d has determined that Customer %d has both their application and picture completed\n", 
        sizeof("Passport clerk %d has determined that Customer %d has both their application and picture completed\n"), id*1000+(Customers[PassportClerks[id].currentCustomer].ssn));
      Signal(PassportClerks[id].lock, PassportClerks[id].cv);
      Release(PassportClerks[id].lock);
      for(i =20; i<100; ++i){
          Yield();
      }
      Acquire(PassportClerks[id].lock);
      Signal(PassportClerks[id].lock, PassportClerks[id].cv);
      Printf("Passport clerk %d has recorded Customer %d passport documentation\n", sizeof("Passport clerk %d has recorded Customer %d passport documentation\n"), id*1000+(Customers[PassportClerks[id].currentCustomer].ssn));
      
    }

    Wait(PassportClerks[id].lock, PassportClerks[id].cv);

    /*if (SenatorArrived && setUpSenator) {
      SenatorLock->Acquire();
      senatorCV->Wait(SenatorLock);
      SenatorLock->Release();
    }*/

    PassportClerks[id].currentCustomer = -1;
    Release(PassportClerks[id].lock);
  }
  Exit(0);
}

void startCashier() {
  int i, id, random;
  Acquire(DataLock);
  id = numActiveCashiers;
  numActiveCashiers++;
  Release(DataLock);

  while(true) {
    if(numCustomers == 0) break;
    Acquire(CashierLineLock);
    if (SenatorArrived) {

    } else if (queue_size(&Cashiers[id].bribeLine) != 0) {
      Signal(CashierLineLock, Cashiers[id].bribeLineCV);
      CashierMoney += 500;
      Cashiers[id].state = 1;
      Cashiers[id].currentCustomer = queue_pop(&Cashiers[id].bribeLine);
      Printf("Cashier %d has received $500 from Customer %d\n",
        sizeof("Cashier %d has received $500 from Customer %d\n"),
        id*1000+Cashiers[id].currentCustomer);
    } else if (queue_size(&Cashiers[id].line) != 0) {
      Signal(CashierLineLock, Cashiers[id].lineCV);
      Cashiers[id].state = 1;
      Cashiers[id].currentCustomer = queue_pop(&Cashiers[id].line);
      Printf("Cashier %d has signalled customer to come to their counter\n", sizeof("Cashier %d has signalled customer to come to their counter."), id);
    } else {
      Acquire(Cashiers[id].lock);
      Release(CashierLineLock);
      Cashiers[id].state = 2;
      Printf("Cashier_ %d is going on break\n", sizeof("Cashier_ %d is going on break\n"), id);

      Wait(Cashiers[id].lock, Cashiers[id].cv);
      Printf("Cashier_ %d is coming off break\n", sizeof("Cashier_ %d is coming off break\n"), id);
      Signal(Cashiers[id].lock, Cashiers[id].cv);
      Cashiers[id].state = 0;

      Release(Cashiers[id].lock);
      continue;
    }
    Acquire(Cashiers[id].lock);
    Release(CashierLineLock);

    Wait(Cashiers[id].lock, Cashiers[id].cv);

    Printf("Cashier_%d has received SSN from Customer_%d \n", sizeof("Cashier_%d has received SSN from Customer_%d \n"), (id*1000+Cashiers[id].currentCustomer));

    /* 5% chance that passport clerk makes a mistake.*/
    random = Rand(4, 0);
    if (random == 0) {
      Printf("Cashier_%d has received $100 from Customer_%d \n", sizeof("Cashier_%d has received $100 from Customer_%d \n"), (id*1000+Cashiers[id].currentCustomer));
      Write(" before certification. They are to go to the back of the line \n", sizeof(" before certification. They are to go to the back of the line \n"), ConsoleOutput);

      Customers[Cashiers[id].currentCustomer].sendToBackOfLine = true;
      Signal(Cashiers[id].lock, Cashiers[id].cv);
    }
    else {
      Signal(Cashiers[id].lock, Cashiers[id].cv);

      Printf("Cashier_%d has verified that Customer_%d \n", sizeof("Cashier_%d has has verified that Customer_%d \n"), (id*1000+Cashiers[id].currentCustomer));
      Write(" has been certified by a PassportClerk \n", sizeof(" has been certified by a PassportClerk \n"), ConsoleOutput);

      Wait(Cashiers[id].lock, Cashiers[id].cv);
      CashierMoney += 100;

      Printf("Cashier_%d has received the $100 from Customer_%d \n", sizeof("Cashier_%d has received the $100 from Customer_%d \n"), (id*1000+Cashiers[id].currentCustomer));
      Write(" after certification \n", sizeof(" after certification \n"), ConsoleOutput);

      Release(Cashiers[id].lock);
      for(i =20; i<100; ++i){
          Yield();
      }
      Acquire(Cashiers[id].lock);
      Printf("Cashier_%d has provided Customer_%d their completed passport \n", sizeof("Cashier_%d has provided Customer_%d their completed passport \n"), (id*1000+Cashiers[id].currentCustomer));
      Signal(Cashiers[id].lock, Cashiers[id].cv);
      Wait(Cashiers[id].lock, Cashiers[id].cv);
      /*
      std::cout << name << " has recorded that " << currentCustomer->getName();
      std::cout << " has been given their completed passport" << std::endl;
      */
      Printf("Cashier_%d has recoreded that Customer_%d \n", sizeof("Cashier_%d has recoreded that Customer_%d \n"), (id*1000+Cashiers[id].currentCustomer));
      Write(" has been given their completed passport \n", sizeof(" has been given their completed passport \n"), ConsoleOutput);

      Signal(Cashiers[id].lock, Cashiers[id].cv);
    }

    Wait(Cashiers[id].lock, Cashiers[id].cv);

    /*if (SenatorArrived && setUpSenator) {
      SenatorLock->Acquire();
      senatorCV->Wait(SenatorLock);
      SenatorLock->Release();
    }*/

    Cashiers[id].currentCustomer = -1;
    Release(Cashiers[id].lock);
  }
  Exit(0);
}

void initGlobalData() {
}

void initCustomersData() {
  int i;
  for (i = 0; i < NUM_CUSTOMERS; i++) {
    Customers[i].name = "customer_" + i;
    Customers[i].ssn = i;
    Customers[i].money = Rand(4, 0)*500+100;
    Customers[i].app_clerk = false;
    Customers[i].pic_clerk = false;
    Customers[i].passport_clerk = false;
    Customers[i].cashier = false;
    Customers[i].sendToBackOfLine = false;
    Customers[i].isSenator = false;
  }
}

void initClerksData() {
  int i;
  char* name;

  int lineCV;
  int bribeLineCV;
  int senatorCV;

  for (i = 0; i < NUM_APPCLERKS; i++) {
    AppClerks[i].name = "appclerk_" + i;
    AppClerks[i].id = i;
    AppClerks[i].state = 0;
    name = numString("appclerk_lock_", sizeof("appclerk_lock_"), i);
    AppClerks[i].lock = CreateLock(name, sizeof(name));
    name = numString("appclerk_cv_", sizeof("appclerk_lock_"), i);
    AppClerks[i].cv = CreateCV(name, sizeof(name));
    name = numString("appclerk_lineCV_", sizeof("appclerk_lock_"), i);
    AppClerks[i].lineCV = CreateCV(name, sizeof(name));
    name = numString("appclerk_bribeLineCV_", sizeof("appclerk_lock_"), i);
    AppClerks[i].bribeLineCV = CreateCV(name, sizeof(name));
    name = numString("appclerk_SenatorCV_", sizeof("appclerk_lock_"), i);
    AppClerks[i].senatorCV = CreateCV(name, sizeof(name));

    queue_init(&AppClerks[i].line);
    queue_init(&AppClerks[i].bribeLine);
    AppClerks[i].likePicture = false;
  }
  for (i = 0; i < NUM_PICCLERKS; i++) {
    PicClerks[i].name = "Picclerk_" + i;
    PicClerks[i].id = i;
    PicClerks[i].state = 0;
    name = numString("Picclerk_lock_", sizeof("Picclerk_lock_"), i);
    PicClerks[i].lock = CreateLock(name, sizeof(name));
    name = numString("Picclerk_cv_", sizeof("Picclerk_lock_"), i);
    PicClerks[i].cv = CreateCV(name, sizeof(name));
    name = numString("Picclerk_lineCV_", sizeof("Picclerk_lock_"), i);
    PicClerks[i].lineCV = CreateCV(name, sizeof(name));
    name = numString("Picclerk_bribeLineCV_", sizeof("Picclerk_lock_"), i);
    PicClerks[i].bribeLineCV = CreateCV(name, sizeof(name));
    name = numString("Picclerk_SenatorCV_", sizeof("Picclerk_lock_"), i);
    PicClerks[i].senatorCV = CreateCV(name, sizeof(name));

    queue_init(&PicClerks[i].line);
    queue_init(&PicClerks[i].bribeLine);
    PicClerks[i].likePicture = false;
  }
  for (i = 0; i < NUM_PASSPORTCLERKS; i++) {
    PassportClerks[i].name = "Passportclerk_" + i;
    PassportClerks[i].id = i;
    PassportClerks[i].state = 0;
    name = numString("Passportclerk_lock_", sizeof("Passportclerk_lock_"), i);
    PassportClerks[i].lock = CreateLock(name, sizeof(name));
    name = numString("Passportclerk_cv_", sizeof("Passportclerk_lock_"), i);
    PassportClerks[i].cv = CreateCV(name, sizeof(name));
    name = numString("Passportclerk_lineCV_", sizeof("Passportclerk_lock_"), i);
    PassportClerks[i].lineCV = CreateCV(name, sizeof(name));
    name = numString("Passportclerk_bribeLineCV_", sizeof("Passportclerk_lock_"), i);
    PassportClerks[i].bribeLineCV = CreateCV(name, sizeof(name));
    name = numString("Passportclerk_SenatorCV_", sizeof("Passportclerk_lock_"), i);
    PassportClerks[i].senatorCV = CreateCV(name, sizeof(name));

    queue_init(&PassportClerks[i].line);
    queue_init(&PassportClerks[i].bribeLine);
    PassportClerks[i].likePicture = false;
  }
  for (i = 0; i < NUM_CASHIERS; i++) {
    Cashiers[i].name = "Cashier_" + i;
    Cashiers[i].id = i;
    Cashiers[i].state = 0;
    name = numString("Cashier_lock_", sizeof("Cashier_lock_"), i);
    Cashiers[i].lock = CreateLock(name, sizeof(name));
    name = numString("Cashier_cv_", sizeof("Cashier_lock_"), i);
    Cashiers[i].cv = CreateCV(name, sizeof(name));
    name = numString("Cashier_lineCV_", sizeof("Cashier_lock_"), i);
    Cashiers[i].lineCV = CreateCV(name, sizeof(name));
    name = numString("Cashier_bribeLineCV_", sizeof("Cashier_lock_"), i);
    Cashiers[i].bribeLineCV = CreateCV(name, sizeof(name));
    name = numString("Cashier_SenatorCV_", sizeof("Cashier_lock_"), i);
    Cashiers[i].senatorCV = CreateCV(name, sizeof(name));

    queue_init(&Cashiers[i].line);
    queue_init(&Cashiers[i].bribeLine);
    Cashiers[i].likePicture = false;
  }
}

void init() {
  initClerksData();
  initCustomersData();
} 

void fork() {
  int i;
  Printf("in Fork", sizeof("in Fork"), ConsoleOutput);
  for (i = 0; i < NUM_APPCLERKS; ++i) {
    Fork(startAppClerk, "appclerk", sizeof("appclerk"));
  }
  for (i = 0; i < NUM_PICCLERKS; ++i) {
    Fork(startPicClerk, "picclerk", sizeof("picclerk"));
  }
  for (i = 0; i < NUM_PASSPORTCLERKS; ++i) {
    Fork(startPassportClerk, "passportclerk", sizeof("passportclerk"));
  }
  for (i = 0; i < NUM_CASHIERS; ++i) {    
    Fork(startCashier, "cashier", sizeof("cashier"));
  }
  for (i = 0; i < NUM_CUSTOMERS; ++i) {
    Fork(startCustomer, "customer", sizeof("customer"));
  }
  Fork(startManager, "manager", sizeof("manager"));
}

int main() {
  Write("FULL_SIMULATION\n", sizeof("FULL_SIMULATION\n"), ConsoleOutput);
  Printf("Number of Customers = %d\n", sizeof("Number of Customers = %d\n"), NUM_CUSTOMERS);
  Printf("Number of ApplicationClerks = %d\n", sizeof("Number of ApplicationClerks = %d\n"), NUM_APPCLERKS);
  Printf("Number of PictureClerks = %d\n", sizeof("Number of PictureClerks = %d\n"), NUM_PICCLERKS);
  Printf("Number of Cashiers = %d\n", sizeof("Number of Cashiers = %d\n"), NUM_CASHIERS);
  Write("Number of Senators = 0\n\n", sizeof("Number of Senators = 0"), ConsoleOutput);

  init();
  fork();
}
