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

  /* declare each Customer/Clerk */
  numCustomers = CreateMV("numCustomers", sizeof("numCustomers"), 1);
  SetMV(numCustomers, 0, NUM_CUSTOMERS);
  customers = CreateMV("customers", sizeof("customers"), NUM_CUSTOMERS);
  appClerks = CreateMV("appClerks", sizeof("appClerks"), NUM_APPCLERKS);
  picClerks = CreateMV("picClerks", sizeof("picClerks"), NUM_PICCLERKS);
  passportClerks = CreateMV("passportClerks", sizeof("passportClerks"), NUM_PASSPORTCLERKS);
  cashiers = CreateMV("cashiers", sizeof("cashiers"), NUM_CASHIERS);

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

void initCustomers(){
  int i, tempCust, money;

  for(i = 0; i < NUM_CUSTOMERS; i++){
    tempCust = CreateMV(numString("Customer", sizeof("Customer"), i), sizeof("Customer")+3, 3);
    SetMV(customers, i, tempCust);
    SetMV(tempCust, SSN, i); 
    money = Rand(4, 0)*500+100;
    SetMV(tempCust, Money, money);
    SetMV(tempCust, SendToBack, false);
  }
}

void initAppClerks(){
  int i, tempClerk, lock, cv, bribeLineCV, lineCV;

  for(i = 0; i < NUM_APPCLERKS; i++){
    tempClerk = CreateMV(numString("AppClerk", sizeof("AppClerk"), i), 
      sizeof("AppClerk")+3, 10);
    SetMV(appClerks, i, tempClerk); 
    SetMV(tempClerk, ID, i); 
    SetMV(tempClerk, State, AVAIL);

    lock = CreateLock(numString("AppClerkLock", sizeof("AppClerkLock"), i), 
      sizeof("AppClerkLock")+3);
    cv = CreateCV(numString("AppClerkCV", sizeof("AppClerkCV"), i), 
      sizeof("AppClerkCV")+3);
    bribeLineCV = CreateCV(numString("AppClerkBribeLineCV", sizeof("AppClerkBribeLineCV"), i), 
      sizeof("AppClerkBribeLineCV")+3);
    lineCV = CreateCV(numString("AppClerkLineCV", sizeof("AppClerkLineCV"), i), 
      sizeof("AppClerkLineCV")+3);
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

  for(i = 0; i < NUM_PICCLERKS; i++){
    tempClerk = CreateMV(numString("PicClerk", sizeof("PicClerk"), i), sizeof("PicClerk")+3, 10);
    SetMV(picClerks, i, tempClerk); 
    SetMV(tempClerk, ID, i); 
    SetMV(tempClerk, State, AVAIL);

    lock = CreateLock(numString("PicClerkLock", sizeof("PicClerkLock"), i), 
      sizeof("PicClerkLock")+3);
    cv = CreateCV(numString("PicClerkCV", sizeof("PicClerkCV"), i), 
      sizeof("PicClerkCV")+3);
    bribeLineCV = CreateCV(numString("PicClerkBribeLineCV", sizeof("PicClerkBribeLineCV"), i), 
      sizeof("PicClerkBribeLineCV")+3);
    lineCV = CreateCV(numString("PicClerkLineCV", sizeof("PicClerkLineCV"), i), 
      sizeof("PicClerkLineCV")+3);
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

  for(i = 0; i < NUM_PASSPORTCLERKS; i++){
    tempClerk = CreateMV(numString("PassportClerk", sizeof("PassportClerk"), i), sizeof("PassportClerk")+3, 10);
    SetMV(passportClerks, i, tempClerk); 
    SetMV(tempClerk, ID, i); 
    SetMV(tempClerk, State, AVAIL);

    lock = CreateLock(numString("PassportClerkLock", sizeof("PassportClerkLock"), i), 
      sizeof("PassportClerkLock")+3);
    cv = CreateCV(numString("PassportClerkCV", sizeof("PassportClerkCV"), i), 
      sizeof("PassportClerkCV")+3);
    bribeLineCV = CreateCV(numString("PassportClerkBribeLineCV", sizeof("PassportClerkBribeLineCV"), i), 
      sizeof("PassportClerkBribeLineCV")+3);
    lineCV = CreateCV(numString("PassportClerkLineCV", sizeof("PassportClerkLineCV"), i), 
      sizeof("PassportClerkLineCV")+3);
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

  for(i = 0; i < NUM_CASHIERS; i++){
    tempClerk = CreateMV(numString("Cashier", sizeof("Cashier"), i), sizeof("Cashier")+3, 10);
    SetMV(cashiers, i, tempClerk); 
    SetMV(tempClerk, ID, i); 
    SetMV(tempClerk, State, AVAIL);

    lock = CreateLock(numString("CashierLock", sizeof("CashierLock"), i), 
      sizeof("CashierLock")+3);
    cv = CreateCV(numString("CashierCV", sizeof("CashierCV"), i), 
      sizeof("CashierCV")+3);
    bribeLineCV = CreateCV(numString("CashierBribeLineCV", sizeof("CashierBribeLineCV"), i), 
      sizeof("CashierBribeLineCV")+3);
    lineCV = CreateCV(numString("CashierLineCV", sizeof("CashierLineCV"), i), 
      sizeof("CashierLineCV")+3);
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
