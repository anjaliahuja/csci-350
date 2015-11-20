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
int numAppClerks;
int numPicClerks;
int numPassportClerks;
int numCashiers;
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
  AppClerkBribeMoney = CreateMV("AppClerkBribeMoney", sizeof("AppClerkBribeMoney"), 0);
  PicClerkBribeMoney = CreateMV("PicClerkBribeMoney", sizeof("PicClerkBribeMoney"), 0);
  PassportClerkBribeMoney = CreateMV("PassportClerkBribeMoney", sizeof("PassportClerkBribeMoney"), 0);
  CashierMoney = CreateMV("CashierMoney", sizeof("CashierMoney"), 0);

  numCustomers = CreateMV("numCustomers", sizeof("numCustomers"), NUM_CUSTOMERS);
  numAppClerks = CreateMV("numAppClerks", sizeof("numAppClerk"), NUM_APPCLERKS);
  numPicClerks = CreateMV("numPicClerks", sizeof("numPicClerks"), NUM_PICCLERKS);
  numPassportClerks = CreateMV("numPassportClerks", sizeof("numPassportClerks", NUM_PASSPORTCLERKS));
  numCashiers = CreateMV("numCashiers", sizeof("numCashiers"), NUM_CASHIERS);
  manager = CreateMV("manager", sizeof("manager"), 1);

  numActiveCustomers = CreateMV("numActiveCustomers", sizeof("numActiveCustomers"), 0);
  numActiveAppClerks = CreateMV("numActiveAppClerks", sizeof("numActiveAppClerks"), 0);
  numActivePicClerks = CreateMV("numActivePicClerks", sizeof("numActivePicClerks"), 0);
  numActivePassportClerks = CreateMV("numActivePassportClerks", sizeof("numActivePassportClerks"), 0);
  numActiveCashiers = CreateMV("numActiveCashiers", sizeof("numActiveCashiers"), 0);
}

void setup(){
  initGlobalData();
  initCustomers();
  initAppClerk();
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
