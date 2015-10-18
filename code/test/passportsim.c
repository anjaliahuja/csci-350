#include "syscall.h"

typedef int bool;
enum bool {false, true};

#define NULL 0
#define NUM_CUSTOMERS 10
#define NUM_APPCLERKS 2
#define NUM_PICCLERKS 2
#define NUM_PASSPORTCLERKS 2
#define NUM_CASHIERS 2

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
} Clerk;

typedef struct {
  char* name;
} Manager;

/* People */
Customer Customers[NUM_CUSTOMERS];
Clerk AppClerks[NUM_APPCLERKS];
Clerk PicClerks[NUM_PICCLERKS];
Clerk PassportClerks[NUM_PASSPORTCLERKS];
Clerk Cashiers[NUM_CASHIERS];
Manager manager;

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
  for (i = 0; i < (NUM_CUSTOMERS+5)-1; i++) {
    q->array[i] = q->array[i+1];
  }
  q->array[NUM_CUSTOMERS+5] = -1;
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
    random = Rand()%10;
    if (Customers[customer].money >= 600 && AppClerks[my_line].state == 1 && random < 3) {
      queue_push(&AppClerks[my_line].bribeLine, customer);
      /*std::cout << this->name << " has gotten in bribe line for " << AppClerks[my_line]->getName() << std::endl;*/
      Wait(AppClerkLineLock, AppClerks[my_line].bribeLineCV);
      Customers[customer].money -= 500;
    } else {
      if (AppClerks[my_line].state == 1 || AppClerks[my_line].state == 2) {
        queue_push(&AppClerks[my_line].bribeLine, customer);
        /*std::cout << this->name << " has gotten in regular line for " << AppClerks[my_line]->getName() << std::endl;*/
        Wait(AppClerkLineLock, AppClerks[my_line].lineCV);
      } else {
        AppClerks[my_line].currentCustomer = customer;
      }
    }

    AppClerks[my_line].state = 1;
    Release(AppClerkLineLock);

    return my_line;

  } else if (type == 'p') {

  } else if (type == 's') {

  } else if (type == 'c') {

  }
}

void getAppFiled(int my_line, int customer) {
  Acquire(AppClerks[my_line].lock);
  Signal(AppClerks[my_line].lock, AppClerks[my_line].cv);
  /*print out*/
  Wait(AppClerks[my_line].lock, AppClerks[my_line].cv);

  Signal(AppClerks[my_line].lock, AppClerks[my_line].cv);
  Release(AppClerks[my_line].lock);
  Customers[customer].app_clerk = true;
}

void getPicTaken(int my_line, int customer) {

}

void getPassport(int my_line, int customer) {

}

void payCashier(int my_line, int customer) {

}

/* Class functions */
void startCustomer(int id) {
  /* data declaration */
  int task;
  int my_line = -1;
  bool isSen = Customers[id].isSenator;

  /*implementation */
  if (isSen) {

  } else {
    task = Rand()%2;
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

    /*print out*/
    numCustomers--;
  }
}

void startAppClerk(int id) {
  int i;

  while(true) {
    if(numCustomers == 0) {
      break;
    }
    Acquire(AppClerkLineLock);
    if (SenatorArrived) {

    } else if (queue_size(&AppClerks[id].bribeLine) != 0) {
      Signal(AppClerkLineLock, AppClerks[id].bribeLineCV);
      /* print out */
      AppClerkBribeMoney += 500;
      AppClerks[id].state = 1;
      AppClerks[id].currentCustomer = queue_pop(&AppClerks[id].bribeLine);
    } else if (queue_size(&AppClerks[id].line) != 0) {
      Signal(AppClerkLineLock, AppClerks[id].lineCV);
      /* print out */
      AppClerkBribeMoney += 500;
      AppClerks[id].state = 1;
      AppClerks[id].currentCustomer = queue_pop(&AppClerks[id].line);
    } else {
      Acquire(AppClerks[id].lock);
      AppClerks[id].state = 2;
      /*print out*/
      Release(AppClerkLineLock);
      Wait(AppClerks[id].lock, AppClerks[id].cv);
      /*print out*/
      Signal(AppClerks[id].lock, AppClerks[id].cv);
      AppClerks[id].state = 0;

      Release(AppClerks[id].lock);
      continue;
    }
    Acquire(AppClerks[id].lock);
    Release(AppClerkLineLock);

    Wait(AppClerks[id].lock, AppClerks[id].cv);
    /*print out */

    Release(AppClerks[id].lock);
    /*for(i =20; i<100; ++i){
      Yield();
    }*/
    Acquire(AppClerks[id].lock);

    Signal(AppClerks[id].lock, AppClerks[id].cv);
    /*print out */

    Wait(AppClerks[id].lock, AppClerks[id].cv);
    if (SenatorArrived) {
      Acquire(SenatorLock);
      Wait(SenatorLock, AppClerks[id].senatorCV);
      Release(SenatorLock);
    }

    AppClerks[id].currentCustomer = -1;
    Release(AppClerks[id].lock);
  }
}

void startPicClerk(int id) {

}

void startPassportClerk(int id) {

}

void startCashiers(int id) {

}

void managerWakeup(Clerk* clerk) {
  Acquire(clerk->lock);
  Signal(clerk->lock, clerk->cv);
  /* Print out 
  std::cout << "Manager has woken up " << clerk->name << std::endl;
  */
  Wait(clerk->lock, clerk->cv);
  Release(clerk->lock);
}

void startManager() {
  int i, j, total = 0;
  while(true) {
    for(i = 0; i < NUM_APPCLERKS; i++) {
    /*If the clerk is on break, aka their state is 2 and their line has more than 3 people
    Wake up the thread*/
    if(queue_size(&AppClerks[i].line) > 2 || (SenatorArrived)) {
        for(j = 0; j < NUM_APPCLERKS; j++) {
          if(AppClerks[j].state == 2) {
            managerWakeup(&AppClerks[j]);
          }
        }
        break;
      }

      if (i == NUM_APPCLERKS-1) {
        for(j = 0; j < NUM_APPCLERKS; j++) {
          if (queue_size(&AppClerks[j].line) > 0 && AppClerks[j].state == 2) {
            managerWakeup(&AppClerks[j]);
          }
        }
      }
    }
     
    for(i = 0; i < NUM_PICCLERKS; i++) {
     if(queue_size(&PicClerks[i].line) > 2 || (SenatorArrived)) {
          for(j = 0; j < NUM_PICCLERKS; j++) {
            if(PicClerks[j].state == 2) {
              managerWakeup(&PicClerks[j]);
            }
          }
          break;
        }

      if (i == NUM_PICCLERKS-1) {
        for(j = 0; j < NUM_PICCLERKS; j++) {
          if (queue_size(&PicClerks[j].line) > 0 && PicClerks[j] .state == 2) {
              managerWakeup(&PicClerks[j]);
          }
        }
      }
    }

    for(i = 0; i < NUM_PASSPORTCLERKS; i++) {
     if(queue_size(&PassportClerks[i].line) > 2 || (SenatorArrived)) {
        for(j = 0; j < NUM_PASSPORTCLERKS; j++) {
          if(PassportClerks[j].state == 2) {
            managerWakeup(&PassportClerks[j]);
          }
        }
        break;
      }

      if (i == NUM_PASSPORTCLERKS-1) {
        for(j = 0; j < NUM_PASSPORTCLERKS; j++) {
          if (queue_size(&PassportClerks[j].line) > 0 && PassportClerks[j].state == 2) {
            managerWakeup(&PassportClerks[j]);
          }
        }
      }
    }

    for(i = 0; i < NUM_CASHIERS; i++) {
      if(queue_size(&Cashiers[i].line) > 2 || (SenatorArrived)) {
        for(j = 0; j < NUM_CASHIERS; j++) {
          if(Cashiers[j].state == 2) {
            managerWakeup(&Cashiers[j]);
          }
        }
        break;
      }

      if (i == NUM_CASHIERS-1) {
        for(j = 0; j < NUM_CASHIERS; j++) {
          if (queue_size(&Cashiers[j].line) > 0 && Cashiers[j].state== 2) {
            managerWakeup(&Cashiers[j]);
          }
        }
      }
    }

    for(i = 0; i < 1000; i++) {
      /*Yield();*/
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
            managerWakeup(&AppClerks[j]);
          }
        }
      for(j = 0; j < NUM_PICCLERKS; j++) {
          if (PicClerks[j] .state == 2) {
            managerWakeup(&PicClerks[j]);
          }
        }
      for(j = 0; j < NUM_PASSPORTCLERKS; j++) {
          if (PassportClerks[j].state == 2) {
            managerWakeup(&PassportClerks[j]);
          }
        }
      for(j = 0; j < NUM_CASHIERS; j++) {
          if (Cashiers[j] . state == 2) {
            managerWakeup(&Cashiers[j]);
          }
        }
      break;
    }
    
    total = AppClerkBribeMoney + PicClerkBribeMoney + PassportClerkBribeMoney + CashierMoney;
    /*
    std::cout << "Manager has counted a total of " << AppClerkBribeMoney << " for Application Clerks" << std::endl;
    std::cout << "Manager has counted a total of " << PicClerkBribeMoney << " for Picture Clerks" << std::endl;
    std::cout << "Manager has counted a total of " << PassportClerkBribeMoney << " for Passport Clerks" << std::endl;
    std::cout << "Manager has counted a total of " << CashierMoney << " for Cashiers" << std::endl;
    std::cout << "Manager has counted a total of " << total << " for the Passport Office" << std::endl;
    */
  }
}

void init() {

} 

int main() {
  init();
}