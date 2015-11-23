#include "syscall.h"

typedef int bool;
enum bool {false, true};

#define NULL 0

#define NUM_CUSTOMERS 6
#define NUM_APPCLERKS 4
#define NUM_PICCLERKS 2
#define NUM_PASSPORTCLERKS 2
#define NUM_CASHIERS 2


typedef struct {
  int array[NUM_CUSTOMERS + 5];
  int numElements;
  int front;
  int back;
} Queue;

/* Customer MV Indicies */
#define SSN 0;
#define Money 1;
#define SendToBack 2;

/* Locks */
int AppClerkLineLock;
int PicClerkLineLock;
int PassportClerkLineLock;
int CashierLineLock;
int DataLock;

/* Money */
int AppClerkBribeMoney;
int PicClerkBribeMoney;
int PassportClerkBribeMoney;
int CashierMoney;

/* Other values */
int numCustomers;
int customers;
int manager;

int numActiveCustomers;
int numActiveAppClerks;
int numActivePicClerks;
int numActivePassportClerks;
int numActiveCashiers;

void initGlobalData(){
  char* name;

  /* Locks */
  AppClerkLineLock = CreateLock("AppClerkLineLock", sizeof("AppClerkLineLock"));
  PicClerkLineLock = CreateLock("PicClerkLineLock", sizeof("PicClerkLineLock"));
  PassportClerkLineLock = CreateLock("PassportClerkLineLock", sizeof("PassportClerkLineLock"));
  CashierLineLock = CreateLock("CashierLineLock", sizeof("CashierLineLock"));
  DataLock = CreateLock("DataLock", sizeof("DataLock"));
  /* Money */
  AppClerkBribeMoney = CreateMV("AppClerkBribeMoney", sizeof("AppClerkBribeMoney"), 1);
  SetMV(AppClerkBribeMoney, 0, 0);

  PicClerkBribeMoney = CreateMV("PicClerkBribeMoney", sizeof("PicClerkBribeMoney"), 1);
  SetMV(PicClerkBribeMoney, 0, 0);

  PassportClerkBribeMoney = CreateMV("PassportClerkBribeMoney", sizeof("PassportClerkBribeMoney"), 1);
  SetMV(PassportClerkBribeMoney, 0, 0);

  CashierMoney = CreateMV("CashierMoney", sizeof("CashierMoney"), 1);
  SetMV(CashierMoney, 0, 0);

  numCustomers = CreateMV("numCustomers", sizeof("numCustomers"), 1);
  SetMV(numCustomers, 0, 0);
  customers = CreateMV("customers", sizeof("customers"), NUM_CUSTOMERS);
  // manager = CreateMV("manager", sizeof("manager"), 1);

  numActiveCustomers = CreateMV("numActiveCustomers", sizeof("numActiveCustomers"), 1);
  SetMV(numActiveCustomers, 0, 0);

  numActiveAppClerks = CreateMV("numActiveAppClerks", sizeof("numActiveAppClerks"), 1);
  SetMV(numActiveAppClerks, 0, 0);

  numActivePicClerks = CreateMV("numActivePicClerks", sizeof("numActivePicClerks"), 1);
  SetMV(numActivePicClerks, 0, 0);

  numActivePassportClerks = CreateMV("numActivePassportClerks", sizeof("numActivePassportClerks"), 1);
  SetMV(numActivePassportClerks, 0, 0);

  numActiveCashiers = CreateMV("numActiveCashiers", sizeof("numActiveCashiers"), 1);
  SetMV(numActivePicClerks, 0, 0);

}

initCustomers(){
  int i, tempCust, money;

  for(i = 0; i < NUM_CUSTOMERS; i++){
    tempCust = CreateMV(addNumToString("Customer", sizeof("Customer"), i), sizeof("Customer")+3, 3);
    SetMV(customers, i, tempCust); 
    SetMV(tempCust, SSN, i); 
    money = Rand(4, 0)*500+100;
    SetMV(tempCust, Money, money);
    SetMV(tempCust, SendToBack, false);
  }
}

void setup(){
  initGlobalData();
  initCustomers();
}

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
