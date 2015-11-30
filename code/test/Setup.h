#include "syscall.h"

typedef int bool;
enum bool {false, true};

#define NULL -10

#define NUM_CUSTOMERS 2
#define NUM_APPCLERKS 1
#define NUM_PICCLERKS 0
#define NUM_PASSPORTCLERKS 0
#define NUM_CASHIERS 0

/* Customer MV Indicies */
#define SSN 0
#define Money 1
#define SendToBack 2

/* Clerk MV Indices */
#define ID 0
#define State 1
#define Lock 2
#define CV 3
#define LineCV 4
#define LineCount 5
#define BribeLineCV 6
#define BribeLineCount 7
#define CurrentCust 8
/* used only for PictureClerk */
#define LikePicture 9

/* Clerk States */
#define AVAIL 0
#define BUSY 1
#define ONBREAK 2

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
int appClerks;
int picClerks;
int passportClerks;
int cashiers;

int numActiveCustomers;
int numActiveAppClerks;
int numActivePicClerks;
int numActivePassportClerks;
int numActiveCashiers;
char* numString(char* str, int length, int num);


void initGlobalData(){
  /* Locks */
  AppClerkLineLock = CreateLock("AppClerkLineLock", sizeof("AppClerkLineLock"));
  PicClerkLineLock = CreateLock("PicClerkLineLock", sizeof("PicClerkLineLock"));
  PassportClerkLineLock = CreateLock("PassportClerkLineLock", sizeof("PassportClerkLineLock"));
  CashierLineLock = CreateLock("CashierLineLock", sizeof("CashierLineLock"));
  DataLock = CreateLock("DataLock", sizeof("DataLock"));

  /* Money */
  AppClerkBribeMoney = CreateMV("AppClerkBribeMoney", sizeof("AppClerkBribeMoney"), 1);
  PicClerkBribeMoney = CreateMV("PicClerkBribeMoney", sizeof("PicClerkBribeMoney"), 1);
  PassportClerkBribeMoney = CreateMV("PassportClerkBribeMoney", sizeof("PassportClerkBribeMoney"), 1);
  CashierMoney = CreateMV("CashierMoney", sizeof("CashierMoney"), 1);

  /* declare each Customer/Clerk */
  numCustomers = CreateMV("numCustomers", sizeof("numCustomers"), 1);
  customers = CreateMV("customers", sizeof("customers"), NUM_CUSTOMERS);
  appClerks = CreateMV("appClerks", sizeof("appClerks"), NUM_APPCLERKS);
  picClerks = CreateMV("picClerks", sizeof("picClerks"), NUM_PICCLERKS);
  passportClerks = CreateMV("passportClerks", sizeof("passportClerks"), NUM_PASSPORTCLERKS);
  cashiers = CreateMV("cashiers", sizeof("cashiers"), NUM_CASHIERS);
  SetMV(numCustomers, 0, NUM_CUSTOMERS);

  numActiveCustomers = CreateMV("numActiveCustomers", sizeof("numActiveCustomers"), 1);
  numActiveAppClerks = CreateMV("numActiveAppClerks", sizeof("numActiveAppClerks"), 1);
  numActivePicClerks = CreateMV("numActivePicClerks", sizeof("numActivePicClerks"), 1);
  numActivePassportClerks = CreateMV("numActivePassportClerks", sizeof("numActivePassportClerks"), 1);
  numActiveCashiers = CreateMV("numActiveCashiers", sizeof("numActiveCashiers"), 1);

}

void initCustomers(){
  int i, tempCust, money;
  SetMV(numActiveCustomers, 0, 0);

  for(i = 0; i < NUM_CUSTOMERS; i++){
    tempCust = CreateMV(numString("Customer", sizeof("Customer"), i), sizeof("Customer")+4, 3);
    SetMV(customers, i, tempCust);
    SetMV(tempCust, SSN, i); 
    money = Rand(4, 0)*500+100;
    SetMV(tempCust, Money, money);
    SetMV(tempCust, SendToBack, false);
  }
}

void initAppClerks(){
  int i, tempClerk, lock, cv, bribeLineCV, lineCV;
  SetMV(numActiveAppClerks, 0, 0);
  SetMV(AppClerkBribeMoney, 0, 0);

  for(i = 0; i < NUM_APPCLERKS; i++){
    tempClerk = CreateMV(numString("AppClerk", sizeof("AppClerk"), i), 
      sizeof("AppClerk")+4, 10);
    SetMV(appClerks, i, tempClerk); 
    SetMV(tempClerk, ID, i); 
    SetMV(tempClerk, State, AVAIL);

    lock = CreateLock(numString("AppClerkLock", sizeof("AppClerkLock"), i), 
      sizeof("AppClerkLock")+4);
    cv = CreateCV(numString("AppClerkCV", sizeof("AppClerkCV"), i), 
      sizeof("AppClerkCV")+4);
    bribeLineCV = CreateCV(numString("AppClerkBribeLineCV", sizeof("AppClerkBribeLineCV"), i), 
      sizeof("AppClerkBribeLineCV")+4);
    lineCV = CreateCV(numString("AppClerkLineCV", sizeof("AppClerkLineCV"), i), 
      sizeof("AppClerkLineCV")+4);
    SetMV(tempClerk, Lock, lock);
    SetMV(tempClerk, CV, cv);
    SetMV(tempClerk, BribeLineCV, bribeLineCV);
    SetMV(tempClerk, LineCV, lineCV);
    SetMV(tempClerk, LineCount, 0);
    SetMV(tempClerk, BribeLineCount, 0);
    SetMV(tempClerk, CurrentCust, NULL);
    SetMV(tempClerk, LikePicture, false);
  }
}

void initPicClerks(){
  int i, tempClerk, lock, cv, bribeLineCV, lineCV;
  SetMV(numActivePicClerks, 0, 0);
  SetMV(PicClerkBribeMoney, 0, 0);

  for(i = 0; i < NUM_PICCLERKS; i++){
    tempClerk = CreateMV(numString("PicClerk", sizeof("PicClerk"), i), sizeof("PicClerk")+4, 10);
    SetMV(picClerks, i, tempClerk); 
    SetMV(tempClerk, ID, i); 
    SetMV(tempClerk, State, AVAIL);

    lock = CreateLock(numString("PicClerkLock", sizeof("PicClerkLock"), i), 
      sizeof("PicClerkLock")+4);
    cv = CreateCV(numString("PicClerkCV", sizeof("PicClerkCV"), i), 
      sizeof("PicClerkCV")+4);
    bribeLineCV = CreateCV(numString("PicClerkBribeLineCV", sizeof("PicClerkBribeLineCV"), i), 
      sizeof("PicClerkBribeLineCV")+4);
    lineCV = CreateCV(numString("PicClerkLineCV", sizeof("PicClerkLineCV"), i), 
      sizeof("PicClerkLineCV")+4);
    SetMV(tempClerk, Lock, lock);
    SetMV(tempClerk, CV, cv);
    SetMV(tempClerk, BribeLineCV, bribeLineCV);
    SetMV(tempClerk, LineCV, lineCV);
    SetMV(tempClerk, LineCount, 0);
    SetMV(tempClerk, BribeLineCount, 0);
    SetMV(tempClerk, CurrentCust, NULL);
    SetMV(tempClerk, LikePicture, false);
  }
}

void initPassportClerks(){
  int i, tempClerk, lock, cv, bribeLineCV, lineCV;
  SetMV(numActivePassportClerks, 0, 0);
  SetMV(PassportClerkBribeMoney, 0, 0);

  for(i = 0; i < NUM_PASSPORTCLERKS; i++){
    tempClerk = CreateMV(numString("PassportClerk", sizeof("PassportClerk"), i), sizeof("PassportClerk")+4, 10);
    SetMV(passportClerks, i, tempClerk); 
    SetMV(tempClerk, ID, i); 
    SetMV(tempClerk, State, AVAIL);

    lock = CreateLock(numString("PassportClerkLock", sizeof("PassportClerkLock"), i), 
      sizeof("PassportClerkLock")+4);
    cv = CreateCV(numString("PassportClerkCV", sizeof("PassportClerkCV"), i), 
      sizeof("PassportClerkCV")+4);
    bribeLineCV = CreateCV(numString("PassportClerkBribeLineCV", sizeof("PassportClerkBribeLineCV"), i), 
      sizeof("PassportClerkBribeLineCV")+4);
    lineCV = CreateCV(numString("PassportClerkLineCV", sizeof("PassportClerkLineCV"), i), 
      sizeof("PassportClerkLineCV")+4);
    SetMV(tempClerk, Lock, lock);
    SetMV(tempClerk, CV, cv);
    SetMV(tempClerk, BribeLineCV, bribeLineCV);
    SetMV(tempClerk, LineCV, lineCV);
    SetMV(tempClerk, LineCount, 0);
    SetMV(tempClerk, BribeLineCount, 0);
    SetMV(tempClerk, CurrentCust, NULL);
    SetMV(tempClerk, LikePicture, false);
  }
}

void initCashiers(){
  int i, tempClerk, lock, cv, bribeLineCV, lineCV;
  SetMV(numActiveCashiers, 0, 0);
  SetMV(CashierMoney, 0, 0);

  for(i = 0; i < NUM_CASHIERS; i++){
    tempClerk = CreateMV(numString("Cashier", sizeof("Cashier"), i), sizeof("Cashier")+4, 10);
    SetMV(cashiers, i, tempClerk); 
    SetMV(tempClerk, ID, i); 
    SetMV(tempClerk, State, AVAIL);

    lock = CreateLock(numString("CashierLock", sizeof("CashierLock"), i), 
      sizeof("CashierLock")+4);
    cv = CreateCV(numString("CashierCV", sizeof("CashierCV"), i), 
      sizeof("CashierCV")+4);
    bribeLineCV = CreateCV(numString("CashierBribeLineCV", sizeof("CashierBribeLineCV"), i), 
      sizeof("CashierBribeLineCV")+4);
    lineCV = CreateCV(numString("CashierLineCV", sizeof("CashierLineCV"), i), 
      sizeof("CashierLineCV")+4);
    SetMV(tempClerk, Lock, lock);
    SetMV(tempClerk, CV, cv);
    SetMV(tempClerk, BribeLineCV, bribeLineCV);
    SetMV(tempClerk, LineCV, lineCV);
    SetMV(tempClerk, LineCount, 0);
    SetMV(tempClerk, BribeLineCount, 0);
    SetMV(tempClerk, CurrentCust, NULL);
    SetMV(tempClerk, LikePicture, false);
  }
}

void setup(){
  initGlobalData();
}

/* Helper Functions */
char* numString(char* str, int length, int num) {
  char cstring[100];

  int i;
  for (i=0; i < length-1; i++) {
    cstring[i] = str[i];
  }
  if(num < 10) {
    cstring[length-1] = (char)(num+'0');
    cstring[length] = '\0';
  } else {
    cstring[length] = (char)((num%10)+'0');
    cstring[length-1] = (char)((num/10)+'0');
    cstring[length+1] = '\0';
  }

  return cstring;
}
