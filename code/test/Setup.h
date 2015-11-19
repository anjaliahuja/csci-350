#include "syscall.h"

typedef int bool;
enum bool {false, true};

#define NULL 0

#define NUM_CUSTOMERS 6
#define NUM_APPCLERKS 4
#define NUM_PICCLERKS 2
#define NUM_PASSPORTCLERKS 2
#define NUM_CASHIERS 2



/*Customer MV Indicies */
#define CustSSN 0
#define CustAppClerkID 1
#define CustPicClerkID 2
#define CustPassClerkID 3
#define CustMoney 4
#define CustBackLine 5

/*App Clerk MV Indicies */
#define ACID 0
#define ACState 1
#define ACLock 2
#define ACCV 3
#define ACLineCV 4
#define ACBribeLineCV 5
#define ACCurrentCust 6

/*Clerk states*/
#define AVAIL 0
#define BUSY 1
#define ONBREAK 2


typedef struct {
  int array[NUM_CUSTOMERS + 5];
  int numElements;
  int front;
  int back;
} Queue;

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

  numCustomers = NUM_CUSTOMERS;

  numActiveCustomers = CreateMV("numActiveCustomers", sizeof("numActiveCustomers"), 0);
  numActiveAppClerks = CreateMV("numActiveAppClerks", sizeof("numActiveAppClerks"), 0);
  numActivePicClerks = CreateMV("numActivePicClerks", sizeof("numActivePicClerks"), 0);
  numActivePassportClerks = CreateMV("numActivePassportClerks", sizeof("numActivePassportClerks"), 0);
  numActiveCashiers = CreateMV("numActiveCashiers", sizeof("numActiveCashiers"), 0);
}

void createAppClerks(){
  appClerks = createMV("appClerks", sizeof("appClerks"), NUM_APPCLERKS);


}

void initAppClerks(){
  int i, lock, cv;

  for(i = 0; i< NUM_APPCLERKS; i++){
    ac = CreateMV(addNumToString("AC", sizeof("AC"), i), sizeof("AC")+3, 7);
    SetMV(appClerks, i, ac); 
    SetMV(ac, ACID, ac); 
    SetMV(ac, ACState, BUSY);

    lock = CreateLock(addNumToString("ACLock", sizeof("ACLock"), i), 
      sizeof("ACLock")+3); 

    SetMV(ac, ACLock, lock); 

    cv = CreateCV(addNumToString("ACCV", sizeof("ACCV"), i), sizeof("ACCV")+3);
    SetMV(ac, ACCV, cv);

    SetMV(ac, ACCurrentCust, -1); 


  }
}

void setup(){
  initGlobalData();
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

char* addNumToString(char* str, int length, int num){
  int i;
  for(i = 0; i< length-1; i++){
    newString[i] = str[i];
  }
  newString[length-1] = '0' + num/100;
  newString[length] = '0' + (num%100) / 10;
  newString[length+1] = '0' + (num%10);
  newString[length+2] = '\0'; 

  return newString; 
}
