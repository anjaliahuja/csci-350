#include "syscall.h"

typedef int bool;
enum bool {false, true};

#define NULL 0

#define NUM_CUSTOMERS 30
#define NUM_APPCLERKS 5
#define NUM_PICCLERKS 5
#define NUM_PASSPORTCLERKS 5
#define NUM_CASHIERS 5

typedef struct {
  int array[NUM_CUSTOMERS + 5];
  int numElements;
  int front;
  int back;
} Queue;

typedef struct {
  char* name;
  int ssn;
  int money;
  bool app_clerk;
  bool pic_clerk;
  bool passport_clerk;
  bool cashier;
  bool sendToBackOfLine;
  bool isSenator;
} Customer;

typedef struct {
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

  /* For PictureClerk only*/
  bool likePicture;
} Clerk;


/* People */
Customer Customers[NUM_CUSTOMERS];
Clerk AppClerks[NUM_APPCLERKS];
Clerk PicClerks[NUM_PICCLERKS];
Clerk PassportClerks[NUM_PASSPORTCLERKS];
Clerk Cashiers[NUM_CASHIERS];

/* Locks */
int AppClerkLineLock;
int PicClerkLineLock;
int PassportClerkLineLock;
int CashierLineLock;

/* Money */
int AppClerkBribeMoney;
int PicClerkBribeMoney;
int PassportClerkBribeMoney;
int CashierMoney;

/* Other values */
bool SenatorArrived;
int SenatorLock;
int numCustomers;

int numActiveCustomers;
int numActiveAppClerks;
int numActivePicClerks;
int numActivePassportClerks;
int numActiveCashiers;

/* Helper Functions */
char cstring[100];
char* numString(char* str, int length, int num) {
  int i;
  for (i=0; i < length; i++) {
    cstring[i] = str[i];
  }
  if(num < 10) {
    cstring[length] = (char)num;
    cstring[length+1] = '\0';
  } else {
    cstring[length+1] = (char)(num%10);
    cstring[length] = (char)(num/10);
    cstring[length+2] = '\0';
  }
  return cstring;
}

/* Implementing a Queue */
void queue_init(Queue* q) {
  int i;
  for (i = 0; i < (NUM_CUSTOMERS+5); i++) {
    q->array[i] = -1;
  }

  q->numElements = 0;
  q->front = -1;
  q->back = -1;
}

int queue_pop(Queue* q) {
  int i;
  int returnVal; 
  if(q->numElements == 0) {
    return -1;
  }
  for (i = 0; i < (NUM_CUSTOMERS+4); i++) {
    q->array[i] = q->array[i+1];
  }
  q->array[NUM_CUSTOMERS+4] = -1;
  returnVal = q->front;
  q->front = q->array[0];
  q->numElements--;
  q->back = q->array[q->numElements-1];
  return returnVal;
}

void queue_push(Queue* q, int e) {
  q->array[q->numElements] = e;
  q->back = e;
  if(q->numElements == 0){
    q->front = e;
  }
  q->numElements++;
}

int queue_size(Queue* q) {
  return q->numElements;
}

int findLine(char type, bool isSenator, int customer) {
  /* variable declarations */
  int my_line = -1;
  int line_size = 9999;
  int i;
  int random = 0;

  /* pass in a char to decide which type of line to find
    'a' is AppClerk, 'p' is PicClerk, 's' is PassportClerk, 'c' is cashier
  */
  if(type == 'a') {
    if(isSenator) {
      AppClerks[0].state = 0;
      AppClerks[0].currentCustomer = customer;
    }

    /* Picking a line */
    Acquire(AppClerkLineLock);
    for(i = 0; i < NUM_APPCLERKS; i++) {
      if(queue_size(&AppClerks[i].line) < line_size) {
        line_size = queue_size(&AppClerks[i].line);
        my_line = i;
      }
    }

    /* Bribe */
    random = Rand(9, 0);
    if (Customers[customer].money >= 600 && AppClerks[my_line].state == 1 && random < 3) {
      queue_push(&AppClerks[my_line].bribeLine, customer);
      Printf("Customer %d has gotten in bribe line for ApplicationClerk %d\n", 
        sizeof("Customer %d has gotten in bribe line for ApplicationClerk %d\n"),
        Customers[customer].ssn*1000+AppClerks[my_line].id);
      Wait(AppClerkLineLock, AppClerks[my_line].bribeLineCV);
      Customers[customer].money -= 500;
    } else {
      if (AppClerks[my_line].state == 1 || AppClerks[my_line].state == 2) {
        queue_push(&AppClerks[my_line].line, customer);
        Printf("Customer %d has gotten in regular line for ApplicationClerk %d\n", 
          sizeof("Customer %d has gotten in regular line for ApplicationClerk %d\n"), 
          Customers[customer].ssn*1000+AppClerks[my_line].id);
        Wait(AppClerkLineLock, AppClerks[my_line].lineCV);
      } else {
        AppClerks[my_line].currentCustomer = customer;
      }
    }

    AppClerks[my_line].state = 1;
    Release(AppClerkLineLock);

    return my_line;
  } else if (type == 'p') {
    if(isSenator) {
      PicClerks[0].state = 0;
      PicClerks[0].currentCustomer = customer;
    }

    /* Picking a line */
    Acquire(PicClerkLineLock);
    for(i = 0; i < NUM_PICCLERKS; i++) {
      if(queue_size(&PicClerks[i].line) < line_size) {
        line_size = queue_size(&PicClerks[i].line);
        my_line = i;
      }
    }

    /* Bribe */
    random = Rand(9, 0);
    if (Customers[customer].money >= 600 && PicClerks[my_line].state == 1 && random < 3) {
      queue_push(&PicClerks[my_line].bribeLine, customer);
      Printf("Customer %d has gotten in bribe line for PictureClerk %d\n", 
        sizeof("Customer %d has gotten in bribe line for PictureClerk %d\n"),
        Customers[customer].ssn*1000+PicClerks[my_line].id);      
      Wait(PicClerkLineLock, PicClerks[my_line].bribeLineCV);
      Customers[customer].money -= 500;
    } else {
      if (PicClerks[my_line].state == 1 || PicClerks[my_line].state == 2) {
        queue_push(&PicClerks[my_line].line, customer);
        Printf("Customer %d has gotten in regular line for PictureClerk %d\n", 
          sizeof("Customer %d has gotten in regular line for PictureClerk %d\n"), 
          Customers[customer].ssn*1000+PicClerks[my_line].id);
        Wait(PicClerkLineLock, PicClerks[my_line].lineCV);
      } else {
        PicClerks[my_line].currentCustomer = customer;
      }
    }

    PicClerks[my_line].state = 1;
    Release(PicClerkLineLock);

    return my_line;
  } else if (type == 's') {
    if(isSenator) {
      PassportClerks[0].state = 0;
      PassportClerks[0].currentCustomer = customer;
    }

    /* Picking a line */
    Acquire(PassportClerkLineLock);
    for(i = 0; i < NUM_PASSPORTCLERKS; i++) {
      if(queue_size(&PassportClerks[i].line) < line_size) {
        line_size = queue_size(&PassportClerks[i].line);
        my_line = i;
      }
    }

    /* Bribe */
    random = Rand(9, 0);
    if (Customers[customer].money >= 600 && PassportClerks[my_line].state == 1 && random < 3) {
      queue_push(&PassportClerks[my_line].bribeLine, customer);
      Printf("Customer %d has gotten in bribe line for PassportClerk %d\n", 
        sizeof("Customer %d has gotten in bribe line for PassportClerk %d\n"),
        Customers[customer].ssn*1000+PassportClerks[my_line].id);      
      Wait(PassportClerkLineLock, PassportClerks[my_line].bribeLineCV);
      Customers[customer].money -= 500;
    } else {
      if (PassportClerks[my_line].state == 1 || PassportClerks[my_line].state == 2) {
        queue_push(&PassportClerks[my_line].line, customer);
        Printf("Customer %d has gotten in regular line for PassportClerk %d\n", 
          sizeof("Customer %d has gotten in regular line for PassportClerk %d\n"), 
          Customers[customer].ssn*1000+PassportClerks[my_line].id);
        Wait(PassportClerkLineLock, PassportClerks[my_line].lineCV);
      } else {
        PassportClerks[my_line].currentCustomer = customer;
      }
    }

    PassportClerks[my_line].state = 1;
    Release(PassportClerkLineLock);

    return my_line;
  } else if (type == 'c') {
    if(isSenator) {
      Cashiers[0].state = 0;
      Cashiers[0].currentCustomer = customer;
    }

    /* Picking a line */
    Acquire(CashierLineLock);
    for(i = 0; i < NUM_CASHIERS; i++) {
      if(queue_size(&Cashiers[i].line) < line_size) {
        line_size = queue_size(&Cashiers[i].line);
        my_line = i;
      }
    }

    /* Bribe */
    random = Rand(9, 0);
    if (Customers[customer].money >= 600 && Cashiers[my_line].state == 1 && random < 3) {
      queue_push(&Cashiers[my_line].bribeLine, customer);
      Printf("Customer %d has gotten in bribe line for Cashier %d\n", 
        sizeof("Customer %d has gotten in bribe line for Cashier %d\n"),
        Customers[customer].ssn*1000+Cashiers[my_line].id);
      Wait(CashierLineLock, Cashiers[my_line].bribeLineCV);
      Customers[customer].money -= 500;
    } else {
      if (Cashiers[my_line].state == 1 || Cashiers[my_line].state == 2) {
        queue_push(&Cashiers[my_line].line, customer);
        Printf("Customer %d has gotten in regular line for Cashier %d\n", 
          sizeof("Customer %d has gotten in regular line for Cashier %d\n"), 
          Customers[customer].ssn*1000+Cashiers[my_line].id);
        Wait(CashierLineLock, Cashiers[my_line].lineCV);
      } else {
        Cashiers[my_line].currentCustomer = customer;
      }
    }

    Cashiers[my_line].state = 1;
    Release(CashierLineLock);

    return my_line;
  }
}

void getAppFiled(int my_line, int customer) {
  Acquire(AppClerks[my_line].lock);
  Signal(AppClerks[my_line].lock, AppClerks[my_line].cv);
  Printf("Customer %d has given SSN %d to ApplicationClerk %d\n",
    sizeof("Customer %d has given SSN %d to ApplicationClerk %d\n"),
    customer*1000000+customer*1000+AppClerks[my_line].id);
  Wait(AppClerks[my_line].lock, AppClerks[my_line].cv);

  Signal(AppClerks[my_line].lock, AppClerks[my_line].cv);
  Release(AppClerks[my_line].lock);
  Customers[customer].app_clerk = true;
}

void getPicTaken(int my_line, int customer) {
  int dislikePicture = 0, i;
  Acquire(PicClerks[my_line].lock);

  Signal(PicClerks[my_line].lock, PicClerks[my_line].cv);
  Printf("Customer %d has given SSN %d to PictureClerk %d\n", sizeof("Customer %d has given SSN %d to PictureClerk %d\n"), customer*1000000+customer*1000+PicClerks[my_line].id);

  /* Waits for PictureClerk to take picture */
  Wait(PicClerks[my_line].lock, PicClerks[my_line].cv);
  /* dislikePicture = Rand () % 100 + 1;*/ /*chooses percentage between 1-99*/
  if (dislikePicture > 50)  {
    Printf("Customer %d does not like their picture from PicClerk %d\n", sizeof("Customer %d does not like their picture from PicClerk %d\n"), customer*1000+PicClerks[my_line].id);

      PicClerks[my_line].likePicture = false;
      Signal(PicClerks[my_line].lock, PicClerks[my_line].cv);
      /* Has to get back in a line.
         Wait to make sure that clerk acknowledges & then go back in line
      */
      Wait(PicClerks[my_line].lock, PicClerks[my_line].cv);

      /*Signal clerk that I'm leaving*/
      Signal(PicClerks[my_line].lock, PicClerks[my_line].cv);
      
      /* Implemented so that the if the clerk they were at can become available again
         Not necessarily punishment
      */
      Release(PicClerks[my_line].lock);
      for(i =100; i<1000; ++i){
          Yield();
      }

      getPicTaken(findLine('p', Customers[customer].isSenator , customer), customer);
      return;
  }

  PicClerks[my_line].likePicture = true;

 Printf("Customer %d likes their picture from PicClerk %d\n", sizeof("Customer %d likes their picture from PicClerk %d\n"), customer*1000+PicClerks[my_line].id);


  /* 
  Signal clerk and Wait to make sure that clerk acknowledges.
  */
  Signal(PicClerks[my_line].lock, PicClerks[my_line].cv);
  Wait(PicClerks[my_line].lock, PicClerks[my_line].cv);

  /* Signal clerk that I'm leaving */
  Signal(PicClerks[my_line].lock, PicClerks[my_line].cv);
  Customers[customer].pic_clerk = true;
  Release(PicClerks[my_line].lock);  
}

void getPassport(int my_line, int customer) {
  int i;
  Acquire(PassportClerks[my_line].lock);
  Signal(PassportClerks[my_line].lock, PassportClerks[my_line].cv);

  Printf("Customer %d has given SSN %d to PassportClerk %d\n", sizeof("Customer %d has given SSN %d to PassportClerk %d\n"), customer*1000000+customer*1000+PassportClerks[my_line].id);
 
  /* Wait to determine whether they go back in line */
  Wait(PassportClerks[my_line].lock, PassportClerks[my_line].cv);

  if(Customers[customer].sendToBackOfLine){
    Printf("Customer %d has gone to PassportClerk %d too soon. They are going to back of line\n", sizeof("Customer %d has gone to PassportClerk %d too soon. They are going to back of line\n"), customer*1000+PassportClerks[my_line].id);
   

    /* Signal clerk that I'm leaving */
    Signal(PassportClerks[my_line].lock, PassportClerks[my_line].cv);
    Release(PassportClerks[my_line].lock);
    for(i =100; i<1000; ++i){
        Yield();
    }

    Customers[customer].sendToBackOfLine = false;
    /* Assuming back of line does not mean their current line */
    getPassport(findLine('s', Customers[customer].isSenator, customer), customer);
    return;
  } 
  
  /* waits for passport clerk to record passport */
  Wait(PassportClerks[my_line].lock, PassportClerks[my_line].cv);    
  
  /* Signal clerk that I'm leaving */
  Signal(PassportClerks[my_line].lock, PassportClerks[my_line].cv);
  Customers[customer].passport_clerk = true;
  Release(PassportClerks[my_line].lock);  
}

void payCashier(int my_line, int customer) {
  int i;

  Acquire(Cashiers[my_line].lock);
  Signal(Cashiers[my_line].lock, Cashiers[my_line].cv);

  Printf("Customer_%d has given SSN %d to Cashier_%d \n", sizeof("Customer_%d has given SSN %d to Cashier_%d \n"), (customer+Customers[customer].ssn*1000+my_line*1000000));

  /* Wait to determine whether they go back in line */
  Wait(Cashiers[my_line].lock, Cashiers[my_line].cv);

  if(Customers[customer].sendToBackOfLine){
    /* send customer to back of line after yield */
    Printf("Customer_%d has gone to Cashier_%d too soon \n", sizeof("Customer_%d has gone to Cashier_%d too soon \n"), (customer+my_line*1000));
    Write("They are going to the back of the line \n", sizeof("They are going to the back of the line \n"), ConsoleOutput);

    /* Signal cashier that I'm leaving */
    Signal(Cashiers[my_line].lock, Cashiers[my_line].cv);

    Release(Cashiers[my_line].lock);
    for(i =100; i<1000; ++i){
        Yield();
    }

    Customers[customer].sendToBackOfLine = false;
    payCashier(findLine('c', Customers[customer].isSenator, customer), customer);
    return;
  }
  
  Signal(Cashiers[my_line].lock, Cashiers[my_line].cv);
  Customers[customer].money -= 100;
  /*if (test6) MONEY += 100;*/
  
  Printf("Customer_%d has given Cashier_%d $100 \n", sizeof("Customer_%d has given Cashier_%d $100 \n"), (customer+my_line*1000));

  /* waits for cashier to give me passport */
  Wait(Cashiers[my_line].lock, Cashiers[my_line].cv);

  /* ensure cashier that i've been given my passport */
  Signal(Cashiers[my_line].lock, Cashiers[my_line].cv);
  Wait(Cashiers[my_line].lock, Cashiers[my_line].cv);

  /* Signal cashier that I'm leaving */
  Signal(Cashiers[my_line].lock, Cashiers[my_line].cv);
  Release(Cashiers[my_line].lock);  
}

/* Class functions */
void startCustomer() {
  int task, id;
  int my_line = -1;
  bool isSen = Customers[id].isSenator;
  id = numActiveCustomers;
  numActiveCustomers++;

  /*implementation */
  if (isSen) {

  } else {
    task = Rand(1, 0);
    if (task == 0){
      my_line = findLine('a', isSen, id);
      getAppFiled(my_line, id);
      my_line = findLine('p', isSen, id);
      getPicTaken(my_line, id);
    } else if (task == 1) {
      my_line = findLine('p', isSen, id);
      getPicTaken(my_line, id);
      my_line = findLine('a', isSen, id);
      getAppFiled(my_line, id);
    }

    my_line = findLine('s', isSen, id);
    getPassport(my_line, id);
    my_line = findLine('c', isSen, id);
    payCashier(my_line, id);

    Printf("Customer %d is leaving the Passport Office\n", 
      sizeof("Customer %d is leaving the Passport Office\n"), 
      id);
    numCustomers--;
  }
  Exit(0);
}

void startAppClerk() {
  int i, id;
  id = numActiveAppClerks;
  numActiveAppClerks++;

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
  id = numActivePicClerks;
  numActivePicClerks++;

  while(true) {
    if (numCustomers == 0) break;
    Acquire(PicClerkLineLock);
    Write("Stop", sizeof("Stop"), ConsoleOutput);


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
  id = numActivePassportClerks;
  numActivePassportClerks++;

  while(true) {
    if(numCustomers == 0) break;
    Acquire(PassportClerkLineLock);
    if (SenatorArrived) {

    } else if (queue_size(&PassportClerks[id].bribeLine) != 0) {
      Signal(PassportClerkLineLock, PassportClerks[id].bribeLineCV);
      Printf("PassportClerk %d has received $500 from Customer %d\n",
        sizeof("PassportClerk %d has received $500 from Customer %d\n"),
        id*1000+PassportClerks[id].currentCustomer);
      PassportClerkBribeMoney += 500;
      PassportClerks[id].state = 1;
      PassportClerks[id].currentCustomer = queue_pop(&PassportClerks[id].bribeLine);
    } else if (queue_size(&PassportClerks[id].line) != 0) {
      Signal(PassportClerkLineLock, PassportClerks[id].lineCV);
      Printf("Passport clerk %d has signalled customer to come to their counter\n", sizeof("Passport clerk %d has signalled customer to come to their counter."), id);
      PassportClerks[id].state = 1;
      PassportClerks[id].currentCustomer = queue_pop(&PassportClerks[id].line);
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
  id = numActiveCashiers;
  numActiveCashiers++;

  while(true) {
    if(numCustomers == 0) break;
    Acquire(CashierLineLock);
    if (SenatorArrived) {

    } else if (queue_size(&Cashiers[id].bribeLine) != 0) {
      Signal(CashierLineLock, Cashiers[id].bribeLineCV);
      Printf("Cashier %d has received $500 from Customer %d\n",
        sizeof("Cashier %d has received $500 from Customer %d\n"),
        id*1000+Cashiers[id].currentCustomer);
      CashierMoney += 500;
      Cashiers[id].state = 1;
      Cashiers[id].currentCustomer = queue_pop(&Cashiers[id].bribeLine);
    } else if (queue_size(&Cashiers[id].line) != 0) {
      Signal(CashierLineLock, Cashiers[id].lineCV);
      Printf("Cashier %d has signalled customer to come to their counter\n", sizeof("Cashier %d has signalled customer to come to their counter."), id);
      Cashiers[id].state = 1;
      Cashiers[id].currentCustomer = queue_pop(&Cashiers[id].line);
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

    Printf("Cashier_%d has received SSN from Customer_%d \n", sizeof("Cashier_%d has received SSN from Customer_%d \n"), (id+Cashiers[id].currentCustomer*1000));

    /* 5% chance that passport clerk makes a mistake.*/
    random = Rand(4, 0);
    if (random == 0) {
      Printf("Cashier_%d has received $100 from Customer_%d \n", sizeof("Cashier_%d has received $100 from Customer_%d \n"), (id+Cashiers[id].currentCustomer*1000));
      Write(" before certification. They are to go to the back of the line \n", sizeof(" before certification. They are to go to the back of the line \n"), ConsoleOutput);

      Customers[Cashiers[id].currentCustomer].sendToBackOfLine = true;
      Signal(Cashiers[id].lock, Cashiers[id].cv);
    }
    else {
      Signal(Cashiers[id].lock, Cashiers[id].cv);

      Printf("Cashier_%d has verified that Customer_%d \n", sizeof("Cashier_%d has has verified that Customer_%d \n"), (id+Cashiers[id].currentCustomer*1000));
      Write(" has been certified by a PassportClerk \n", sizeof(" has been certified by a PassportClerk \n"), ConsoleOutput);

      Wait(Cashiers[id].lock, Cashiers[id].cv);
      CashierMoney += 100;

      Printf("Cashier_%d has received the $100 from Customer_%d \n", sizeof("Cashier_%d has received the $100 from Customer_%d \n"), (id+Cashiers[id].currentCustomer*1000));
      Write(" after certification \n", sizeof(" after certification \n"), ConsoleOutput);

      Release(Cashiers[id].lock);
      for(i =20; i<100; ++i){
          Yield();
      }
      Acquire(Cashiers[id].lock);
      Printf("Cashier_%d has provided Customer_%d their completed passport \n", sizeof("Cashier_%d has provided Customer_%d their completed passport \n"), (id+Cashiers[id].currentCustomer*1000));
      Signal(Cashiers[id].lock, Cashiers[id].cv);
      Wait(Cashiers[id].lock, Cashiers[id].cv);
      /*
      std::cout << name << " has recorded that " << currentCustomer->getName();
      std::cout << " has been given their completed passport" << std::endl;
      */
      Printf("Cashier_%d has recoreded that Customer_%d \n", sizeof("Cashier_%d has recoreded that Customer_%d \n"), (id+Cashiers[id].currentCustomer*1000));
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

void managerWakeupAppClerk(Clerk* clerk) {
  Acquire(clerk->lock);
  Signal(clerk->lock, clerk->cv);
  Printf("Manager has woken up ApplicationClerk %d \n", sizeof("Manager has woken up ApplicationClerk %d \n"), (clerk->id));
  Wait(clerk->lock, clerk->cv);
  Release(clerk->lock);
}
void managerWakeupPicClerk(Clerk* clerk) {
  Acquire(clerk->lock);
  Signal(clerk->lock, clerk->cv);
  Printf("Manager has woken up PictureClerk %d \n", sizeof("Manager has woken up PictureClerk %d \n"), (clerk->id));
  Wait(clerk->lock, clerk->cv);
  Release(clerk->lock);
}
void managerWakeupPassportClerk(Clerk* clerk) {
  Acquire(clerk->lock);
  Signal(clerk->lock, clerk->cv);
  Printf("Manager has woken up PassportClerk %d \n", sizeof("Manager has woken up PassportClerk %d \n"), (clerk->id));
  Wait(clerk->lock, clerk->cv);
  Release(clerk->lock);
}
void managerWakeupCashier(Clerk* clerk) {
  Acquire(clerk->lock);
  Signal(clerk->lock, clerk->cv);
  Printf("Manager has woken up Cashier %d \n", sizeof("Manager has woken up Cashier %d \n"), (clerk->id));
  Wait(clerk->lock, clerk->cv);
  Release(clerk->lock);
}

void startManager() {
  int i, j, total = 0;
  while(true) {
    for(i = 0; i < NUM_APPCLERKS; i++) {
    /*If the clerk is on break, aka their state is 2 and their line has more than 3 people
    Wake up the thread*/
    if(queue_size(&AppClerks[i].line) > 2 || SenatorArrived) {
        for(j = 0; j < NUM_APPCLERKS; j++) {
          if(AppClerks[j].state == 2 || 
            (i == NUM_APPCLERKS && queue_size(&AppClerks[j].line) > 0 && AppClerks[j].state == 2)) {
            managerWakeupAppClerk(&AppClerks[j]);
          }
        }
        break;
      }

      if (i == NUM_APPCLERKS-1) {
        for(j = 0; j < NUM_APPCLERKS; j++) {
          if (queue_size(&AppClerks[j].line) > 0 && AppClerks[j].state == 2) {
            managerWakeupAppClerk(&AppClerks[j]);
          }
        }
      }
    }
     
    for(i = 0; i < NUM_PICCLERKS; i++) {
     if(queue_size(&PicClerks[i].line) > 2 || (SenatorArrived)) {
          for(j = 0; j < NUM_PICCLERKS; j++) {
            if(PicClerks[j].state == 2) {
              managerWakeupPicClerk(&PicClerks[j]);
            }
          }
          break;
        }

      if (i == NUM_PICCLERKS-1) {
        for(j = 0; j < NUM_PICCLERKS; j++) {
          if (queue_size(&PicClerks[j].line) > 0 && PicClerks[j] .state == 2) {
              managerWakeupPicClerk(&PicClerks[j]);
          }
        }
      }
    }

    for(i = 0; i < NUM_PASSPORTCLERKS; i++) {
     if(queue_size(&PassportClerks[i].line) > 2 || (SenatorArrived)) {
        for(j = 0; j < NUM_PASSPORTCLERKS; j++) {
          if(PassportClerks[j].state == 2) {
            managerWakeupPassportClerk(&PassportClerks[j]);
          }
        }
        break;
      }

      if (i == NUM_PASSPORTCLERKS-1) {
        for(j = 0; j < NUM_PASSPORTCLERKS; j++) {
          if (queue_size(&PassportClerks[j].line) > 0 && PassportClerks[j].state == 2) {
            managerWakeupPassportClerk(&PassportClerks[j]);
          }
        }
      }
    }

    for(i = 0; i < NUM_CASHIERS; i++) {
      if(queue_size(&Cashiers[i].line) > 2 || (SenatorArrived)) {
        for(j = 0; j < NUM_CASHIERS; j++) {
          if(Cashiers[j].state == 2) {
            managerWakeupCashier(&Cashiers[j]);
          }
        }
        break;
      }

      if (i == NUM_CASHIERS-1) {
        for(j = 0; j < NUM_CASHIERS; j++) {
          if (queue_size(&Cashiers[j].line) > 0 && Cashiers[j].state== 2) {
            managerWakeupCashier(&Cashiers[j]);
          }
        }
      }
    }

    for(i = 0; i < 1000; i++) {
      Yield();
    }

    /*Add code for checking amount of money we have*/
    /*if (SenatorArrived) {
      sem.V();
      continue;
    }
    if (test1) continue;*/

    /* no more customers */
    if (numCustomers == 0) {
      for(j = 0; j < NUM_APPCLERKS; j++) {
          if(AppClerks[j].state == 2) {
            managerWakeupAppClerk(&AppClerks[j]);
          }
        }
      for(j = 0; j < NUM_PICCLERKS; j++) {
          if (PicClerks[j] .state == 2) {
            managerWakeupPicClerk(&PicClerks[j]);
          }
        }
      for(j = 0; j < NUM_PASSPORTCLERKS; j++) {
          if (PassportClerks[j].state == 2) {
            managerWakeupPassportClerk(&PassportClerks[j]);
          }
        }
      for(j = 0; j < NUM_CASHIERS; j++) {
          if (Cashiers[j] . state == 2) {
            managerWakeupCashier(&Cashiers[j]);
          }
        }
    total = AppClerkBribeMoney + PicClerkBribeMoney + PassportClerkBribeMoney + CashierMoney;

    Printf("Manager has has counted a total of %d for Application Clerks \n", sizeof("Manager has has counted a total of %d for Application Clerks \n"), AppClerkBribeMoney);
    Printf("Manager has has counted a total of %d for Picture Clerks \n", sizeof("Manager has has counted a total of %d for Picture Clerks \n"), PicClerkBribeMoney);
    Printf("Manager has has counted a total of %d for Passport Clerks \n", sizeof("Manager has has counted a total of %d for Passport Clerks \n"), PassportClerkBribeMoney);
    Printf("Manager has has counted a total of %d for Cashiers \n", sizeof("Manager has has counted a total of %d for Cashiers\n"), CashierMoney);
    Printf("Manager has has counted a total of %d for the Passport Office \n", sizeof("Manager has has counted a total of %d for the Passport Office \n"), total);
    
      break;
    }
    
         }
    total = AppClerkBribeMoney + PicClerkBribeMoney + PassportClerkBribeMoney + CashierMoney;

    Printf("Manager has has counted a total of %d for Application Clerks \n", sizeof("Manager has has counted a total of %d for Application Clerks \n"), AppClerkBribeMoney);
    Printf("Manager has has counted a total of %d for Picture Clerks \n", sizeof("Manager has has counted a total of %d for Picture Clerks \n"), PicClerkBribeMoney);
    Printf("Manager has has counted a total of %d for Passport Clerks \n", sizeof("Manager has has counted a total of %d for Passport Clerks \n"), PassportClerkBribeMoney);
    Printf("Manager has has counted a total of %d for Cashiers \n", sizeof("Manager has has counted a total of %d for Cashiers\n"), CashierMoney);
    Printf("Manager has has counted a total of %d for the Passport Office \n", sizeof("Manager has has counted a total of %d for the Passport Office \n"), total);
    
  }
  Exit(0);
}

void initGlobalData() {
  char* name;

  /* Locks */
  AppClerkLineLock = CreateLock("AppClerkLineLock", sizeof("AppClerkLineLock"));
  PicClerkLineLock = CreateLock("PicClerkLineLock", sizeof("PicClerkLineLock"));
  PassportClerkLineLock = CreateLock("PassportClerkLineLock", sizeof("PassportClerkLineLock"));
  CashierLineLock = CreateLock("CashierLineLock", sizeof("CashierLineLock"));

  /* Money */
  AppClerkBribeMoney = 0;
  PicClerkBribeMoney = 0;
  PassportClerkBribeMoney = 0;
  CashierMoney = 0;

  /* Other values */
  SenatorArrived = false;
  SenatorLock = CreateLock("SenatorLock", sizeof("SenatorLock"));;
  numCustomers = NUM_CUSTOMERS;

  numActiveCustomers = 0;
  numActiveAppClerks = 0;
  numActivePicClerks = 0;
  numActivePassportClerks = 0;
  numActiveCashiers = 0;
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
  initGlobalData();
  initClerksData();
  initCustomersData();
} 

void fork() {
  int i;
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
  init();
  fork();
}
