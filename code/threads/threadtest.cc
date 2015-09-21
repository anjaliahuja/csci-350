//  Simple test cases for the threads assignment.
//

#include "copyright.h"
#include "system.h"
#ifdef CHANGED
#include "synch.h"
#include <iostream>
#endif

#ifdef CHANGED
// --------------------------------------------------
// Test Suite
// --------------------------------------------------


// --------------------------------------------------
// Test 1 - see TestSuite() for details
// --------------------------------------------------
Semaphore t1_s1("t1_s1",0);       // To make sure t1_t1 acquires the
                                  // lock before t1_t2
Semaphore t1_s2("t1_s2",0);       // To make sure t1_t2 Is waiting on the 
                                  // lock before t1_t3 releases it
Semaphore t1_s3("t1_s3",0);       // To make sure t1_t1 does not release the
                                  // lock before t1_t3 tries to acquire it
Semaphore t1_done("t1_done",0);   // So that TestSuite knows when Test 1 is
                                  // done
Lock t1_l1("t1_l1");          // the lock tested in Test 1

// --------------------------------------------------
// t1_t1() -- test1 thread 1
//     This is the rightful lock owner
// --------------------------------------------------
void t1_t1() {
    t1_l1.Acquire();
    t1_s1.V();  // Allow t1_t2 to try to Acquire Lock
 
    printf ("%s: Acquired Lock %s, waiting for t3\n",currentThread->getName(),
        t1_l1.getName());
    t1_s3.P();
    printf ("%s: working in CS\n",currentThread->getName());
    for (int i = 0; i < 1000000; i++) ;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
        t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t2() -- test1 thread 2
//     This thread will wait on the held lock.
// --------------------------------------------------
void t1_t2() {

    t1_s1.P();  // Wait until t1 has the lock
    t1_s2.V();  // Let t3 try to acquire the lock

    printf("%s: trying to acquire lock %s\n",currentThread->getName(),
        t1_l1.getName());
    t1_l1.Acquire();

    printf ("%s: Acquired Lock %s, working in CS\n",currentThread->getName(),
        t1_l1.getName());
    for (int i = 0; i < 10; i++)
    ;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
        t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t3() -- test1 thread 3
//     This thread will try to release the lock illegally
// --------------------------------------------------
void t1_t3() {

    t1_s2.P();  // Wait until t2 is ready to try to acquire the lock

    t1_s3.V();  // Let t1 do it's stuff
    for ( int i = 0; i < 3; i++ ) {
    printf("%s: Trying to release Lock %s\n",currentThread->getName(),
           t1_l1.getName());
    t1_l1.Release();
    }
}

// --------------------------------------------------
// Test 2 - see TestSuite() for details
// --------------------------------------------------
Lock t2_l1("t2_l1");        // For mutual exclusion
Condition t2_c1("t2_c1");   // The condition variable to test
Semaphore t2_s1("t2_s1",0); // To ensure the Signal comes before the wait
Semaphore t2_done("t2_done",0);     // So that TestSuite knows when Test 2 is
                                  // done

// --------------------------------------------------
// t2_t1() -- test 2 thread 1
//     This thread will signal a variable with nothing waiting
// --------------------------------------------------
void t2_t1() {
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
       t2_l1.getName(), t2_c1.getName());
    t2_c1.Signal(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t2_l1.getName());
    t2_l1.Release();
    t2_s1.V();  // release t2_t2
    t2_done.V();
}

// --------------------------------------------------
// t2_t2() -- test 2 thread 2
//     This thread will wait on a pre-signalled variable
// --------------------------------------------------
void t2_t2() {
    t2_s1.P();  // Wait for t2_t1 to be done with the lock
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
       t2_l1.getName(), t2_c1.getName());
    t2_c1.Wait(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t2_l1.getName());
    t2_l1.Release();
}
// --------------------------------------------------
// Test 3 - see TestSuite() for details
// --------------------------------------------------
Lock t3_l1("t3_l1");        // For mutual exclusion
Condition t3_c1("t3_c1");   // The condition variable to test
Semaphore t3_s1("t3_s1",0); // To ensure the Signal comes before the wait
Semaphore t3_done("t3_done",0); // So that TestSuite knows when Test 3 is
                                // done

// --------------------------------------------------
// t3_waiter()
//     These threads will wait on the t3_c1 condition variable.  Only
//     one t3_waiter will be released
// --------------------------------------------------
void t3_waiter() {
    t3_l1.Acquire();
    t3_s1.V();      // Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
       t3_l1.getName(), t3_c1.getName());
    t3_c1.Wait(&t3_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t3_c1.getName());
    t3_l1.Release();
    t3_done.V();
}


// --------------------------------------------------
// t3_signaller()
//     This threads will signal the t3_c1 condition variable.  Only
//     one t3_signaller will be released
// --------------------------------------------------
void t3_signaller() {

    // Don't signal until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ ) 
    t3_s1.P();
    t3_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
       t3_l1.getName(), t3_c1.getName());
    t3_c1.Signal(&t3_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t3_l1.getName());
    t3_l1.Release();
    t3_done.V();
}
 
// --------------------------------------------------
// Test 4 - see TestSuite() for details
// --------------------------------------------------
Lock t4_l1("t4_l1");        // For mutual exclusion
Condition t4_c1("t4_c1");   // The condition variable to test
Semaphore t4_s1("t4_s1",0); // To ensure the Signal comes before the wait
Semaphore t4_done("t4_done",0); // So that TestSuite knows when Test 4 is
                                // done

// --------------------------------------------------
// t4_waiter()
//     These threads will wait on the t4_c1 condition variable.  All
//     t4_waiters will be released
// --------------------------------------------------
void t4_waiter() {
    t4_l1.Acquire();
    t4_s1.V();      // Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
       t4_l1.getName(), t4_c1.getName());
    t4_c1.Wait(&t4_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t4_c1.getName());
    t4_l1.Release();
    t4_done.V();
}


// --------------------------------------------------
// t2_signaller()
//     This thread will broadcast to the t4_c1 condition variable.
//     All t4_waiters will be released
// --------------------------------------------------
void t4_signaller() {

    // Don't broadcast until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ ) 
    t4_s1.P();
    t4_l1.Acquire();
    printf("%s: Lock %s acquired, broadcasting %s\n",currentThread->getName(),
       t4_l1.getName(), t4_c1.getName());
    t4_c1.Broadcast(&t4_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t4_l1.getName());
    t4_l1.Release();
    t4_done.V();
}
// --------------------------------------------------
// Test 5 - see TestSuite() for details
// --------------------------------------------------
Lock t5_l1("t5_l1");        // For mutual exclusion
Lock t5_l2("t5_l2");        // Second lock for the bad behavior
Condition t5_c1("t5_c1");   // The condition variable to test
Semaphore t5_s1("t5_s1",0); // To make sure t5_t2 acquires the lock after
                                // t5_t1

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a condition under t5_l1
// --------------------------------------------------
void t5_t1() {
    t5_l1.Acquire();
    t5_s1.V();  // release t5_t2
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
       t5_l1.getName(), t5_c1.getName());
    t5_c1.Wait(&t5_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a t5_c1 condition under t5_l2, which is
//     a Fatal error
// --------------------------------------------------
void t5_t2() {
    t5_s1.P();  // Wait for t5_t1 to get into the monitor
    t5_l1.Acquire();
    t5_l2.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
       t5_l2.getName(), t5_c1.getName());
    t5_c1.Signal(&t5_l2);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t5_l2.getName());
    t5_l2.Release();
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// TestSuite()
//     This is the main thread of the test suite.  It runs the
//     following tests:
//
//       1.  Show that a thread trying to release a lock it does not
//       hold does not work
//
//       2.  Show that Signals are not stored -- a Signal with no
//       thread waiting is ignored
//
//       3.  Show that Signal only wakes 1 thread
//
//   4.  Show that Broadcast wakes all waiting threads
//
//       5.  Show that Signalling a thread waiting under one lock
//       while holding another is a Fatal error
//
//     Fatal errors terminate the thread in question.
// --------------------------------------------------
void TestSuite() {
    Thread *t;
    char *name;
    int i;
    
    // Test 1

    printf("Starting Test 1\n");

    t = new Thread("t1_t1");
    t->Fork((VoidFunctionPtr)t1_t1,0);

    t = new Thread("t1_t2");
    t->Fork((VoidFunctionPtr)t1_t2,0);

    t = new Thread("t1_t3");
    t->Fork((VoidFunctionPtr)t1_t3,0);

    // Wait for Test 1 to complete
    for (  i = 0; i < 2; i++ )
    t1_done.P();

    // Test 2

    printf("Starting Test 2.  Note that it is an error if thread t2_t2\n");
    printf("completes\n");

    t = new Thread("t2_t1");
    t->Fork((VoidFunctionPtr)t2_t1,0);

    t = new Thread("t2_t2");
    t->Fork((VoidFunctionPtr)t2_t2,0);

    // Wait for Test 2 to complete
    t2_done.P();

    // Test 3

    printf("Starting Test 3\n");

    for (  i = 0 ; i < 5 ; i++ ) {
    name = new char [20];
    sprintf(name,"t3_waiter%d",i);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)t3_waiter,0);
    }
    t = new Thread("t3_signaller");
    t->Fork((VoidFunctionPtr)t3_signaller,0);

    // Wait for Test 3 to complete
    for (  i = 0; i < 2; i++ )
    t3_done.P();

    // Test 4

    printf("Starting Test 4\n");

    for (  i = 0 ; i < 5 ; i++ ) {
    name = new char [20];
    sprintf(name,"t4_waiter%d",i);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)t4_waiter,0);
    }
    t = new Thread("t4_signaller");
    t->Fork((VoidFunctionPtr)t4_signaller,0);

    // Wait for Test 4 to complete
    for (  i = 0; i < 6; i++ )
    t4_done.P();

    // Test 5

    printf("Starting Test 5.  Note that it is an error if thread t5_t1\n");
    printf("completes\n");

    t = new Thread("t5_t1");
    t->Fork((VoidFunctionPtr)t5_t1,0);

    t = new Thread("t5_t2");
    t->Fork((VoidFunctionPtr)t5_t2,0);

}
#endif

#include <queue>

//Code for our Passport Office Simulation
//By: Anjali Ahuja, Anne Kao, and Bernard Xie, Started 09/13/15
//
//Declaring Global Variables

//Application Clerk Line
Lock* AppClerkLineLock;
int AppClerkBribeMoney;

//Picture Clerk Line
Lock* PicClerkLineLock;
int PicClerkBribeMoney;

//Passport Clerk Line
Lock* PassportClerkLineLock;
int PassportClerkBribeMoney;

//Cashier Line
Lock* CashierLineLock;
// App money + bribe money
int CashierMoney;


Semaphore sem("sem", 0);

int NUM_APP_CLERKS;
int NUM_PIC_CLERKS;
int NUM_PASSPORT_CLERKS;
int NUM_CASHIERS;
int NUM_CUSTOMERS;

// For tests.  Have to sem.V() for test to finish.
bool test1;
bool test6;
int MONEY;


//Defining Classes

//Customer class
class Customer : public Thread{
  public:
    //custom constructor
    Customer(char* debugName, int id) : Thread(debugName) {
      name = debugName;
      ssn = id; // pass in from test
      app_clerk = false;
      pic_clerk = false;
      passport_clerk = false;
      cashier = false;
      sendToBackOfLine = false;
      money = rand()%4*500 + 100;
    }

    void CustomerStart();
    int FindAppLine();
    void GetApplicationFiled(int my_line);
    int FindPicLine();
    void GetPictureTaken(int my_line);
    int FindPassportLine();
    void GetPassport(int my_line);

    int FindCashierLine();
    void PayCashier(int my_line);

    bool is_app_completed() {
      return app_clerk && pic_clerk && passport_clerk;
    }
    int get_money() {
      return money;
    }

    int get_ssn() {
      return ssn;
    }

    void set_app_clerk() {
      app_clerk = true;
    }
    void set_pic_clerk() {
      pic_clerk = true;
    }
    void set_passport_clerk() {
      passport_clerk = true;
    }
    void set_cashier() {
      cashier = true;
    }
    bool get_app_clerk() {
      return app_clerk;
    }
    bool get_pic_clerk() {
      return pic_clerk;
    }
    bool get_passport_clerk() {
      return passport_clerk;
    }
    bool get_cashier() {
      return cashier;
    }

    int getSSN() {
      return ssn;
    }

    void setSendToBackOfLine() {
      sendToBackOfLine = true;
    }

  private:
    char* name;
    int ssn;
    bool app_clerk;
    bool pic_clerk;
    bool passport_clerk;
    bool cashier;
    bool sendToBackOfLine;
    int money;
};

class AppClerk : public Thread {
  public:
    AppClerk(char* debugName, int id) : Thread(debugName) {
      name = debugName;
      this->id = id;
      state = 0; //0 is available, 1 is busy, 2 is on break 
      totalServiced = 0;
      lock = new Lock(debugName);
      cv = new Condition(debugName);
      lineCV = new Condition(debugName);
      bribeLineCV = new Condition(debugName);

      currentCustomer = NULL;
      line = new std::queue<Customer*>();
      bribeLine = new std::queue<Customer*>();
    }

    ~AppClerk() {
      delete lock;
      delete cv;
      delete lineCV;
      delete bribeLineCV;
      delete line;
      delete bribeLine;
    }

    void AppClerkStart() { 
      while(true) {
        AppClerkLineLock->Acquire();

        if (getBribeLineSize() != 0) {
          bribeLineCV->Signal(AppClerkLineLock);
          currentCustomer = bribeLine->front();
          std::cout << name << " has received $500 from " << currentCustomer->getName() << std::endl;
          AppClerkBribeMoney += 500;
          this->state = 1;
          bribeLine->pop();
        } else if(getLineSize() != 0) {
          lineCV->Signal(AppClerkLineLock);
          currentCustomer = line->front();
          std::cout << name << " has signalled a Customer to come to their counter" << std::endl;
          this->state = 1;
          line->pop();
        } else {
          //TODO: Add code for on break
          this->state = 2;
          std::cout << name << " is going on break" << std::endl;
          AppClerkLineLock->Release();
          this->lock->Acquire();
          cv->Wait(this->lock);
          std::cout << name << " has woken up" << std::endl;
          // Signal manager that i'm awake
          cv->Signal(this->lock);
          this->lock->Release();
          this->state = 0;
          continue;
        }

        AppClerkLineLock->Release();
        this->lock->Acquire();

        //Wait for customer data
        cv->Wait(this->lock);
        std::cout << name << " has received SSN " << currentCustomer->getSSN();
        std::cout << " from " << currentCustomer->getName() << std::endl;

        // File application
        lock->Release(); // no busy waiting
        for(int i =20; i<100; ++i){
            currentThread->Yield();
        }
        lock->Acquire();

        // Application filed
        cv->Signal(this->lock);
        std::cout << name << " has recorded a completed application for ";
        std::cout << currentCustomer->getName() << std::endl;
        this->totalServiced++;

        // Wait for customer to leave
        cv->Wait(this->lock);
        currentCustomer = NULL;
        this->lock->Release();
      }
    }

    void Acquire() {
      this->lock->Acquire();
    }

    void Release() {
      this->lock->Release();
    }

    int getState() {
      return this->state;
    }

    void setState(int s) {
      this->state = s;
    } 

    int getLineSize() {
      return line->size();
    }

    int getBribeLineSize() {
      return bribeLine->size();
    }

    int getTotalServiced() {
        return totalServiced;
    }

    Lock* getLock() {
        return lock;
    }

    Condition* getCV() {
        return cv;
    }

    Condition* getLineCV() {
      return lineCV;
    }

    Condition* getBribeLineCV() {
      return bribeLineCV;
    }

    char* getName() {
        return name;
    }

    void addToLine(Customer* customer) {
      line->push(customer);
    }

    void addToBribeLine(Customer* customer) {
      bribeLine->push(customer);
    }

    void setCurrentCustomer(Customer* customer) {
      currentCustomer = customer;
    }

  private:
    char* name;
    int id;
    int state;
    int totalServiced;
    Lock* lock;
    Condition* cv;
    Condition* lineCV;
    Condition* bribeLineCV;

    Customer* currentCustomer;
    std::queue<Customer*>* line;
    std::queue<Customer*>* bribeLine;
};


class PicClerk : public Thread {
  public:
    PicClerk(char* debugName, int id) : Thread(debugName) {
      name = debugName;
      this->id = id;
      state = 0; //0 is available, 1 is busy, 2 is on break
      totalServiced = 0;
      lock = new Lock(debugName);
      cv = new Condition(debugName);
      lineCV = new Condition(debugName);
      bribeLineCV = new Condition(debugName);

      currentCustomer = NULL;
      line = new std::queue<Customer*>();
      bribeLine = new std::queue<Customer*>();
    }

    ~PicClerk() {
      delete lock;
      delete cv;
      delete lineCV;
      delete bribeLineCV;
      delete line;
      delete bribeLine;
    }

    void PicClerkStart() {
      while(true) {
        PicClerkLineLock->Acquire();

        if (getBribeLineSize() != 0) {
          bribeLineCV->Signal(PicClerkLineLock);
          currentCustomer = bribeLine->front();
          PicClerkBribeMoney += 500;
          this->state = 1;
          bribeLine->pop();
        } else if(getLineSize() != 0) {
          lineCV->Signal(PicClerkLineLock);
          currentCustomer = line->front();
          std::cout << name << " has signalled a Customer to come to their counter" << std::endl;
          this->state = 1;
          line->pop();
        } else {
                    //TODO: Add code for on break
          this->state = 2;
          std::cout << name << " is going on break" << std::endl;
          PicClerkLineLock->Release();
          this->lock->Acquire();
          cv->Wait(this->lock);
          std::cout << name << " has woken up" << std::endl;
          cv->Signal(this->lock);
          this->lock->Release();
          this->state = 0;
          continue;
        }

        this->lock->Acquire();
        PicClerkLineLock->Release();

         //Wait for customer data
        cv->Wait(this->lock);
        std::cout << name << " has received SSN " << currentCustomer->getSSN();
        std::cout << " from " << currentCustomer->getName() << std::endl;

        //Do my job, customer now waiting
        cv->Signal(this->lock);
        std::cout << name << " has taken a picture of " << currentCustomer->getName() << std::endl;

        //waiting for approval
        cv->Wait(this->lock);

        if (!likePicture) {
          std::cout << name << " has been told that " << currentCustomer->getName();
          std::cout << " does not like their picture" << std::endl;
          // Signals customer to go to the back of the line
          cv->Signal(this->lock);
        }
        else {
          std::cout << name << " has been told that " << currentCustomer->getName();
          std::cout << " does like their picture" << std::endl;

          // file picture
          lock->Release(); // no busy waiting
          for(int i =20; i<100; ++i){
              currentThread->Yield();
          }
          lock->Acquire();

          //picture filed
          cv->Signal(this->lock);
          totalServiced++;
        }

        // Wait for customer to leave
        cv->Wait(this->lock);
        currentCustomer = NULL;
        this->lock->Release();
      }
    }

    void Acquire() {
      this->lock->Acquire();
    }

    void Release() {
      this->lock->Release();
    }

    int getState() {
      return this->state;
    }

    void setState(int s) {
      this->state = s;
    } 

    int getLineSize() {
      // line includes currentCustomer
      return line->size();
    }

    int getBribeLineSize() {
      return bribeLine->size();
    }

    int getTotalServiced() {
        return totalServiced;
    }

    Lock* getLock() {
        return lock;
    }

    Condition* getCV() {
        return cv;
    }

    Condition* getLineCV() {
      return lineCV;
    }

    Condition* getBribeLineCV() {
      return bribeLineCV;
    }

    char* getName() {
        return name;
    }

    void addToLine(Customer* customer) {  
      line->push(customer);
    }

    void addToBribeLine(Customer* customer) {
      bribeLine->push(customer);
    }

    void setLikePicture(bool like) {
      likePicture = like;
    }

    void setCurrentCustomer(Customer* customer) {
      currentCustomer = customer;
    }

  private:
    char* name;
    int id;
    int state;
    int totalServiced;
    bool likePicture;
    Lock* lock;
    Condition* cv;
    Condition* lineCV;
    Condition* bribeLineCV;

    Customer* currentCustomer;
    std::queue<Customer*>* line;
    std::queue<Customer*>* bribeLine;
};

class PassportClerk : public Thread {
  public:
    PassportClerk(char* debugName, int id) : Thread(debugName) {
      name = debugName;
      this->id = id;
      state = 0; //0 is available, 1 is busy, 2 is on break
      lineSize = 0;
      bribeLineSize = 0;
      totalServiced = 0;
      lock = new Lock(debugName);
      cv = new Condition(debugName);
      lineCV = new Condition(debugName);
      bribeLineCV = new Condition(debugName);

      currentCustomer = NULL;
      line = new std::queue<Customer*>();
      bribeLine = new std::queue<Customer*>();
    }

    ~PassportClerk() {
      delete lock;
      delete cv;
      delete lineCV;
      delete bribeLineCV;
      delete line;
      delete bribeLine;
    }

    void PassportClerkStart() {
      while(true) {
        PassportClerkLineLock->Acquire();

        if (bribeLine->size() != 0) {
          bribeLineCV->Signal(PassportClerkLineLock);
          currentCustomer = bribeLine->front();
          PassportClerkBribeMoney += 500;
          this->state = 1;
          bribeLine->pop();
        } else if(line->size() != 0) {
          lineCV->Signal(PassportClerkLineLock);
          currentCustomer = line->front();
          std::cout << name << " has signalled a Customer to come to their counter" << std::endl;
          this->state = 1;
          line->pop();
        } else {
                    //TODO: Add code for on break
          this->state = 2;
          std::cout << name << " is going on break" << std::endl;
          PassportClerkLineLock->Release();
          this->lock->Acquire();
          cv->Wait(this->lock);
          std::cout << name << " has woken up" << std::endl;
          cv->Signal(this->lock);
          this->lock->Release();
          this->state = 0;
          continue;
        }

        PassportClerkLineLock->Release();
        this->lock->Acquire();

        // Wait for customer to come to counter.
        cv->Wait(this->lock);
        std::cout << name << " has received SSN " << currentCustomer->getSSN();
        std::cout << " from " << currentCustomer->getName() << std::endl;

        // 5% chance that passport clerk makes a mistake.
        int random = rand()%20;
        if (random == 0) {
          std::cout << name << " has determined that " << currentCustomer->getName();
          std::cout << " does not have both their application and picture completed" << std::endl;
          // Signals customer to go to the back of the line
          // make customer wait
          currentCustomer->setSendToBackOfLine();
          cv->Signal(this->lock);
        }
        else {
          std::cout << name << " has determined that " << currentCustomer->getName();
          std::cout << " has both their application and picture completed" << std::endl;
          cv->Signal(this->lock);

          // Record customer
          lock->Release(); // no busy waiting
          for(int i =20; i<100; ++i){
              currentThread->Yield();
          }
          lock->Acquire();

          // customer recorded
          cv->Signal(this->lock);
          std::cout << name << " has recorded " << currentCustomer->getName() << " passport documentation" << std::endl;
          totalServiced++;
        }

        // Wait for customer to leave
        cv->Wait(this->lock);
        currentCustomer = NULL;
        this->lock->Release();
      }
    }
 
    void Acquire() {
      this->lock->Acquire();
    }

    void Release() {
      this->lock->Release();
    }

    int getState() {
      return this->state;
    }

     void setState(int s) {
      this->state = s;
    } 

    int getLineSize() {
      return line->size();
    }

    int getBribeLineSize() {
      return bribeLine->size();
    }

    int getTotalServiced() {
        return totalServiced;
    }

    Lock* getLock() {
        return lock;
    }

    Condition* getCV() {
        return cv;
    }

    Condition* getLineCV() {
      return lineCV;
    }

    Condition* getBribeLineCV() {
      return bribeLineCV;
    }

    char* getName() {
        return name;
    }

    void addToLine(Customer* customer) {
      line->push(customer);
    }

    void addToBribeLine(Customer* customer) {
      bribeLine->push(customer);
    }

    void setCurrentCustomer(Customer* customer) {
      currentCustomer = customer;
    }

  private:
    char* name;
    int id;
    int state;
    int lineSize;
    int bribeLineSize;
    int totalServiced;
    Lock* lock;
    Condition* cv;
    Condition* lineCV;
    Condition* bribeLineCV;

    Customer* currentCustomer;
    std::queue<Customer*>* line;
    std::queue<Customer*>* bribeLine;
};

class Cashier : public Thread {
  public:
     Cashier(char* debugName, int id) : Thread(debugName) {
      name = debugName;
      this->id = id;
      state = 0; //0 is available, 1 is busy, 2 is on break
      lineSize = 0;
      bribeLineSize = 0;
      totalServiced = 0;
      lock = new Lock(debugName);
      cv = new Condition(debugName);
      lineCV = new Condition(debugName);
      bribeLineCV = new Condition(debugName);

      currentCustomer = NULL;
      line = new std::queue<Customer*>();
      bribeLine = new std::queue<Customer*>();
    }
    void CashierStart() {
      while(true) {
        CashierLineLock->Acquire();

        if (bribeLine->size() != 0) {
          bribeLineCV->Signal(CashierLineLock);
          currentCustomer = bribeLine->front();
          // bribe money goes to cashier received money amount
          CashierMoney += 500;
          this->state = 1;
          bribeLine->pop();
        } else if(line->size() != 0) {
          lineCV->Signal(CashierLineLock);
          currentCustomer = line->front();
          std::cout << name << " has signalled a Customer to come to their counter" << std::endl;
          this->state = 1;
          line->pop();
        } else {
                    //TODO: Add code for on break
          this->state = 2;
          std::cout << name << " is going on break" << std::endl;
          CashierLineLock->Release();
          this->lock->Acquire();
          cv->Wait(this->lock);
          std::cout << name << " has woken up" << std::endl;
          cv->Signal(this->lock);
          this->lock->Release();
          this->state = 0;
          continue;
        }

        CashierLineLock->Release();
        this->lock->Acquire();

        // Wait for customer to come to counter.
        cv->Wait(this->lock);
        std::cout << name << " has received SSN " << currentCustomer->getSSN();
        std::cout << " from " << currentCustomer->getName() << std::endl;

        // 5% chance that passport clerk makes a mistake.
        int random = rand()%20;
        if (random == 0) {
          // Don't actually take the money
          std::cout << name << " has received the $100 from " << currentCustomer->getName();
          std::cout << " before certification. They are to go to the back of the line" << std::endl;
          // Signals customer to go to the back of the line
          // make customer wait
          currentCustomer->setSendToBackOfLine();
          cv->Signal(this->lock);
        }
        else {
          // Verify customer
          cv->Signal(this->lock);
          std::cout << name << " has verified that " << currentCustomer->getName();
          std::cout << " has been certified by a PassportClerk" << std::endl;

          // Wait for money
          cv->Wait(this->lock);
          CashierMoney += 100;
          std::cout << name << " has received the $100 from " << currentCustomer->getName();
          std::cout << " after certification" << std::endl;

          // Record customer
          lock->Release(); // no busy waiting
          for(int i =20; i<100; ++i){
              currentThread->Yield();
          }
          lock->Acquire();

          cv->Signal(this->lock);
          std::cout << name << " has provided " << currentCustomer->getName();
          std::cout << " their completed passport" << std::endl;

          cv->Wait(this->lock);
          std::cout << name << " has recorded that " << currentCustomer->getName();
          std::cout << " has been given their completed passport" << std::endl;
      
          cv->Signal(this->lock);
          totalServiced++;
        }

        // Wait for customer to leave
        cv->Wait(this->lock);
        currentCustomer = NULL;
        this->lock->Release();
      }
    }

    void Acquire() {
      this->lock->Acquire();
    }

    void Release() {
      this->lock->Release();
    }

    int getState() {
      return this->state;
    }

     void setState(int s) {
      this->state = s;
    } 

    int getLineSize() {
      return line->size();
    }

    int getBribeLineSize() {
      return bribeLine->size();
    }

    int getTotalServiced() {
        return totalServiced;
    }

    Lock* getLock() {
        return lock;
    }

    Condition* getCV() {
        return cv;
    }

    Condition* getLineCV() {
      return lineCV;
    }

    Condition* getBribeLineCV() {
      return bribeLineCV;
    }

    char* getName() {
        return name;
    }

    void addToLine(Customer* customer) {
      line->push(customer);
    }

    void addToBribeLine(Customer* customer) {
      bribeLine->push(customer);
    }

    void setCurrentCustomer(Customer* customer) {
      currentCustomer = customer;
    }

  private:
    char* name;
    int id;
    int state;
    int lineSize;
    int bribeLineSize;
    int totalServiced;
    Lock* lock;
    Condition* cv;
    Condition* lineCV;
    Condition* bribeLineCV;

    Customer* currentCustomer;
    std::queue<Customer*>* line;
    std::queue<Customer*>* bribeLine;
};

class Manager : public Thread{
  public:
    Manager(char* debugName, int id) : Thread(debugName) {
      name = debugName;
      totalMoney = 0;
    }

    void ManagerStart();
  private:
    char* name;
    int totalMoney;

};

// Declaring class global variables.
// List of Clerks
AppClerk** AppClerks;
PicClerk** PicClerks;
PassportClerk** PassportClerks;
Cashier** Cashiers;

//and Manager
Manager* manager;

//List of Customers
Customer** Customers;

void Manager::ManagerStart() {
  //TODO: Implement code for when all clerks are on break.  Wake them all up (or just clerk 0). 
  // Anne: I think we should wake up all of them or else clerk 0 will be the only clerk servicing

  while(true) {
    for(int i = 0; i < NUM_APP_CLERKS; i++) {
      //If the clerk is on break, aka their state is 2 and their line has more than 3 people
      //Wake up the thread
      if(AppClerks[i]->getLineSize() > 2) {
        for(int j = 0; j < NUM_APP_CLERKS; j++) {
          if(AppClerks[i]->getState() == 2) {
            AppClerks[i]->Acquire();
            AppClerks[i]->getCV()->Signal(AppClerks[i]->getLock());
            std::cout << "Manager has woken up " << AppClerks[i]->getName() << std::endl;
            
            // Wait for AppClerk to acknowledge
            AppClerks[i]->getCV()->Wait(AppClerks[i]->getLock());
            AppClerks[i]->Release();
          }
        }
        break;
      }

      if (i == NUM_APP_CLERKS-1) {
        for(int j = 0; j < NUM_APP_CLERKS; j++) {
          if (AppClerks[j]->getLineSize() > 0 && AppClerks[j] ->getState() == 2) {
            AppClerks[j]->Acquire();
            AppClerks[j]->getCV()->Signal(AppClerks[j]->getLock());
            std::cout << "Manager has woken up " << AppClerks[j]->getName() << std::endl;
            
            // Wait for AppClerk to acknowledge
            AppClerks[j]->getCV()->Wait(AppClerks[j]->getLock());
            AppClerks[j]->Release();
          }
        }
      }
    }
     
    for(int i = 0; i < NUM_PIC_CLERKS; i++) {
       if(PicClerks[i]->getLineSize() > 2) {
          for(int j = 0; j < NUM_PIC_CLERKS; j++) {
            if(PicClerks[i]->getState() == 2) {
              PicClerks[i]->Acquire();
              PicClerks[i]->getCV()->Signal(PicClerks[i]->getLock());
              std::cout << "Manager has woken up " << PicClerks[i]->getName() << std::endl;
              
              // Wait for PicClerk to Acknowledge
              PicClerks[i]->getCV()->Wait(PicClerks[i]->getLock());
              PicClerks[i]->Release();
            }
          }
          break;
        }

      if (i == NUM_PIC_CLERKS-1) {
        for(int j = 0; j < NUM_PIC_CLERKS; j++) {
          if (PicClerks[j]->getLineSize() > 0 && PicClerks[j] ->getState() == 2) {
            PicClerks[j]->Acquire();
            PicClerks[j]->getCV()->Signal(PicClerks[j]->getLock());
            std::cout << "Manager has woken up " << PicClerks[j]->getName() << std::endl;
            
            // Wait for AppClerk to acknowledge
            PicClerks[j]->getCV()->Wait(PicClerks[j]->getLock());
            PicClerks[j]->Release();
          }
        }
      }
    }

    for(int i = 0; i < NUM_PASSPORT_CLERKS; i++) {
      if(PassportClerks[i]->getLineSize() > 2) {
        for(int j = 0; j < NUM_PASSPORT_CLERKS; j++) {
          if(PassportClerks[i]->getState() == 2) {
            PassportClerks[i]->Acquire();
            PassportClerks[i]->getCV()->Signal(PassportClerks[i]->getLock());
            std::cout << "Manager has woken up " << PassportClerks[i]->getName() << std::endl;
          
            // Wait for PassportClerk to Acknowledge
            PassportClerks[i]->getCV()->Wait(PassportClerks[i]->getLock());
            PassportClerks[i]->Release();
          }
        }
        break;
      }

      if (i == NUM_PASSPORT_CLERKS-1) {
        for(int j = 0; j < NUM_PASSPORT_CLERKS; j++) {
          if (PassportClerks[j]->getLineSize() > 0 && PassportClerks[j] ->getState() == 2) {
            PassportClerks[j]->Acquire();
            PassportClerks[j]->getCV()->Signal(PassportClerks[j]->getLock());
            std::cout << "Manager has woken up " << PassportClerks[j]->getName() << std::endl;
            
            // Wait for AppClerk to acknowledge
            PassportClerks[j]->getCV()->Wait(PassportClerks[j]->getLock());
            PassportClerks[j]->Release();
          }
        }
      }
    }

    for(int i = 0; i < NUM_CASHIERS; i++) {
      if(Cashiers[i]->getLineSize() > 2) {
        for(int j = 0; j < NUM_CASHIERS; j++) {
          if(Cashiers[i]->getState() == 2) {
            Cashiers[i]->Acquire();
            Cashiers[i]->getCV()->Signal(Cashiers[i]->getLock());
            std::cout << "Manager has woken up " << Cashiers[i]->getName() << std::endl;

            // Wait for Cashier to acknowledge
            Cashiers[i]->getCV()->Wait(Cashiers[i]->getLock());
            Cashiers[i]->Release();
          }
        }
        break;
      }

      if (i == NUM_CASHIERS-1) {
        for(int j = 0; j < NUM_CASHIERS; j++) {
          if (Cashiers[j]->getLineSize() > 0 && Cashiers[j] ->getState() == 2) {
            Cashiers[j]->Acquire();
            Cashiers[j]->getCV()->Signal(Cashiers[j]->getLock());
            std::cout << "Manager has woken up " << Cashiers[j]->getName() << std::endl;
            
            // Wait for AppClerk to acknowledge
            Cashiers[j]->getCV()->Wait(Cashiers[j]->getLock());
            Cashiers[j]->Release();
          }
        }
      }
    }

    for(int i = 0; i < 100; i++) {
      currentThread->Yield();
    }

    //Add code for checking amount of money we have
    // int AppClerkBribeMoney;
    // int PicClerkBribeMoney;
    // int PassportClerkBribeMoney;
    // int CashierMoney;

    /*int total = AppClerkBribeMoney + PicClerkBribeMoney + PassportClerkBribeMoney + CashierMoney;
    std::cout << "Manager has counted a total of " << AppClerkBribeMoney << " for Application Clerks" << std::endl;
    std::cout << "Manager has counted a total of " << PicClerkBribeMoney << " for Picture Clerks" << std::endl;
    std::cout << "Manager has counted a total of " << PassportClerkBribeMoney << " for Passport Clerks" << std::endl;
    std::cout << "Manager has counted a total of " << CashierMoney << " for Cashiers" << std::endl;
    std::cout << "Manager has counted a total of " << total << " for the Passport Office" << std::endl;
    */
  }
}

void Customer::CustomerStart() {
  // TODO: Implement random chance it goes to things in the wrong order
  // mainly Passport clerk b4 cashier (i think)
  // Not even entirely sure if this is a requiremetn
  int task = rand()%2;
  int my_line = -10;
  if (task == 0) {
    my_line = FindAppLine();
    GetApplicationFiled(my_line);

    my_line = FindPicLine();
    GetPictureTaken(my_line);
  } else if (task == 1) { // Go to picture line
    my_line = FindPicLine();
    GetPictureTaken(my_line);

    my_line = FindAppLine();
    GetApplicationFiled(my_line);
  }
  
  my_line = FindPassportLine();
  GetPassport(my_line);

  my_line = FindCashierLine();
  PayCashier(my_line);

  std::cout << name << " is leaving the Passport Office" << std::endl;
  sem.V();
}

int Customer::FindAppLine() {
  AppClerkLineLock->Acquire();
  if(test1) 
    std::cout << "TEST_1: " << name << " has acquired ApplicationClerk's line lock" << std::endl;
  int my_line = -1;
  int line_size = 9999;
  for(int i = 0; i < NUM_APP_CLERKS; i++) {
    if(AppClerks[i]->getLineSize() < line_size && AppClerks[i]->getState() != 2) {
      line_size = AppClerks[i]->getLineSize();
      my_line = i;
    }
  }

  if (my_line = -1) {
    line_size = AppClerks[0]->getLineSize();
    my_line = 0;
  }

  // Bribe
  // Only go on bribe line if you have enough money and 
  // the clerk is busy (else you'd be spending $500 for no reason)
  // 30% chance customer will bribe the clerk.
  int random = rand()%10;
  if (money >= 600 && AppClerks[my_line]->getState() == 1
      && random < 3) {
    AppClerks[my_line]->addToBribeLine(this);
    std::cout << this->name << " has gotten in bribe line for " << AppClerks[my_line]->getName() << std::endl;
    if (test1) sem.V();
    AppClerks[my_line]->getBribeLineCV()->Wait(AppClerkLineLock);
    money -= 500;
    if (test6) MONEY += 500;
  } else {
    if (AppClerks[my_line]->getState() == 1 || AppClerks[my_line]->getState() == 2) {
      AppClerks[my_line]->addToLine(this);
      std::cout << this->name << " has gotten in regular line for " << AppClerks[my_line]->getName() << std::endl;
      if (test1) sem.V();
      AppClerks[my_line]->getLineCV()->Wait(AppClerkLineLock);
    } else {
      AppClerks[my_line]->setCurrentCustomer(this);
    }
  }

  AppClerks[my_line]->setState(1);
  AppClerkLineLock->Release();

  return my_line;
}

void Customer::GetApplicationFiled(int my_line) {
  AppClerks[my_line]->Acquire();
  // Give my data to my clerk
  AppClerks[my_line]->getCV()->Signal(AppClerks[my_line]->getLock());
  std::cout << this->name << " has given SSN " << ssn << " to " << AppClerks[my_line]->getName() << std::endl;

  // Wait for clerk to do their job
  AppClerks[my_line]->getCV()->Wait(AppClerks[my_line]->getLock());

  // Signal clerk that I'm leaving
  AppClerks[my_line]->getCV()->Signal(AppClerks[my_line]->getLock());
  AppClerks[my_line]->Release();
  this->app_clerk = true;
  // TODO: what lock is this?
  // lock->Release()
  // currentThread->Sleep();
  // lock->Acquire()
}

int Customer::FindPicLine() {
  PicClerkLineLock->Acquire();
  int my_line = -1;
  int line_size = 9999;
  for(int i = 0; i < NUM_PIC_CLERKS; i++) {
    if(PicClerks[i]->getLineSize() < line_size && PicClerks[i]->getState() != 2) {
      line_size = PicClerks[i]->getLineSize();
      my_line = i;
    }
  }

  if (my_line == -1) {
    line_size = PicClerks[0]->getLineSize();
    my_line = 0;
  }

  // Bribe
  // Only go on bribe line if you have enough money and 
  // the clerk is busy (else you'd be spending $500 for no reason)
  // 30% chance customer will bribe the clerk.
  int random = rand()%10;
  if (money >= 600 && PicClerks[my_line]->getState() == 1
      && random < 3) {
    PicClerks[my_line]->addToBribeLine(this);
    std::cout << this->name << " has gotten in bribe line for " << PicClerks[my_line]->getName() << std::endl;
    PicClerks[my_line]->getBribeLineCV()->Wait(PicClerkLineLock);
    money -= 500;
    if (test6) MONEY += 500;
  } else {
    if (PicClerks[my_line]->getState() == 1 || PicClerks[my_line]->getState() == 2) {
      PicClerks[my_line]->addToLine(this);
      std::cout << this->name << " has gotten in regular line for " << PicClerks[my_line]->getName() << std::endl;
      PicClerks[my_line]->getLineCV()->Wait(PicClerkLineLock);
    } else {
      PicClerks[my_line]->setCurrentCustomer(this);
    }
  }

  PicClerks[my_line]->setState(1);
  PicClerkLineLock->Release();

  return my_line;
}

void Customer::GetPictureTaken(int my_line){
    PicClerks[my_line]->Acquire();

    PicClerks[my_line]->getCV()->Signal(PicClerks[my_line]->getLock());
    std::cout << this->name << " has given SSN " << ssn << " to " << PicClerks[my_line]->getName() << std::endl;

    //Waits for pic clerk to take picture
    PicClerks[my_line]->getCV()->Wait(PicClerks[my_line]->getLock());
    //Checking picture
    int dislikePicture = rand () % 100 + 1; //chooses percentage between 1-99
    if (dislikePicture > 50)  {
        std::cout << this->name << " does not like their picture from " << PicClerks[my_line]->getName() << std::endl;
        PicClerks[my_line]->setLikePicture(false);
        PicClerks[my_line]->getCV()->Signal(PicClerks[my_line]->getLock());
        // // Has to get back in a line.
        // Wait to make sure that clerk acknowledges & then go back in line
        PicClerks[my_line]->getCV()->Wait(PicClerks[my_line]->getLock());

         // Signal clerk that I'm leaving
        PicClerks[my_line]->getCV()->Signal(PicClerks[my_line]->getLock());
        
        // Implemented so that the if the clerk they were at can become available again
        // Not necessarily punishment
        PicClerks[my_line]->Release(); // no busy waiting
        for(int i =100; i<1000; ++i){
            currentThread->Yield();
        }

        GetPictureTaken(FindPicLine());
        return;
    }

    //Customer likes picture enough
    PicClerks[my_line]->setLikePicture(true);
    std::cout << this->name << " does like their picture from " << PicClerks[my_line]->getName() << std::endl;
    // Signal clerk and Wait to make sure that clerk acknowledges.
    PicClerks[my_line]->getCV()->Signal(PicClerks[my_line]->getLock());

    // Wait for application to be complete
    PicClerks[my_line]->getCV()->Wait(PicClerks[my_line]->getLock());

    // Signal clerk that I'm leaving
    PicClerks[my_line]->getCV()->Signal(PicClerks[my_line]->getLock());
    this->pic_clerk = true;
    PicClerks[my_line]->Release();  
}

int Customer::FindPassportLine() {
  PassportClerkLineLock->Acquire();
  int my_line = -1;
  int line_size = 9999;
  for(int i = 0; i < NUM_PASSPORT_CLERKS; i++) {
    if(PassportClerks[i]->getLineSize() < line_size && PassportClerks[i]->getState() != 2) {
      line_size = PassportClerks[i]->getLineSize();
      my_line = i;
    }
  } 

  if (my_line == -1) {
    line_size = PassportClerks[0]->getLineSize();
    my_line = 0;
  }

  // Bribe
  // Only go on bribe line if you have enough money and 
  // the clerk is busy (else you'd be spending $500 for no reason)
  // 30% chance customer will bribe the clerk.
  int random = rand()%10;
  if (money >= 600 && PassportClerks[my_line]->getState() == 1
      && random < 3) {
    PassportClerks[my_line]->addToBribeLine(this);
    std::cout << this->name << " has gotten in bribe line for " << PassportClerks[my_line]->getName() << std::endl;
    PassportClerks[my_line]->getBribeLineCV()->Wait(PassportClerkLineLock);
    money -= 500;
    if (test6) MONEY += 500;
  } else {
    if (PassportClerks[my_line]->getState() == 1 || PassportClerks[my_line]->getState() == 2) {
      PassportClerks[my_line]->addToLine(this);
      std::cout << this->name << " has gotten in regular line for " << PassportClerks[my_line]->getName() << std::endl;
      PassportClerks[my_line]->getLineCV()->Wait(PassportClerkLineLock);
    } else {
      PassportClerks[my_line]->setCurrentCustomer(this);
    }
  }

  PassportClerks[my_line]->setState(1);
  PassportClerkLineLock->Release();

  return my_line;
}

void Customer::GetPassport(int my_line) {
    /*PassportClerks will "certify" a Customer ONLY if the 
    Customer has a filed application and picture.f a Customer shows up
     to the PassportClerk BEFORE both documenets are filed, they they 
     (the Customer) are punished by being forec to wait for some arbitrary amount of time. 
     This is to be from 100 to 1000 currentThread->Yield() calls. 
     After these calls are completed, ,the Customer goes to the back of the PassportClerk line. 
     NOTE It takes time for a PassportClerk to "record" a Customer's completed documents. */

    PassportClerks[my_line]->Acquire();

    // Give SSN
    PassportClerks[my_line]->getCV()->Signal(PassportClerks[my_line]->getLock());
    std::cout << this->name << " has given SSN " << ssn << " to " << PassportClerks[my_line]->getName() << std::endl;

    // Wait to determine whether they go back in line
    PassportClerks[my_line]->getCV()->Wait(PassportClerks[my_line]->getLock());

    if(sendToBackOfLine){
      //send customer to back of line after yield
      std::cout << this->name << " has gone to " << PassportClerks[my_line]->getName() << " too soon. ";
      std::cout << "They are going to the back of the line." << std::endl;

      // Signal clerk that I'm leaving
      PassportClerks[my_line]->getCV()->Signal(PassportClerks[my_line]->getLock());

      // Punishment
      PassportClerks[my_line]->Release(); // no busy waiting
      for(int i =100; i<1000; ++i){
          currentThread->Yield();
      }

      sendToBackOfLine = false;
      // Assuming back of line does not mean their current line
      GetPassport(FindPassportLine());
      return;
    } 
    
    //waits for passport clerk to record passport
    PassportClerks[my_line]->getCV()->Wait(PassportClerks[my_line]->getLock());    
    
    // Signal clerk that I'm leaving
    PassportClerks[my_line]->getCV()->Signal(PassportClerks[my_line]->getLock());
    this->passport_clerk = true;
    PassportClerks[my_line]->Release();  
}

int Customer::FindCashierLine() {
  CashierLineLock->Acquire();
  int my_line = -1;
  int line_size = 9999;
  for(int i = 0; i < NUM_CASHIERS; i++) {
    if(Cashiers[i]->getLineSize() < line_size && Cashiers[i]->getState() != 2) {
      line_size = Cashiers[i]->getLineSize();
      my_line = i;
    }
  }

  if (my_line == -1) {
    line_size = Cashiers[0]->getLineSize();
    my_line = 0;
  }

  // Bribe
  // Only go on bribe line if you have enough money and 
  // the clerk is busy (else you'd be spending $500 for no reason)
  // 30% chance customer will bribe the clerk.
  int random = rand()%10;
  if (money >= 600 && Cashiers[my_line]->getState() == 1
      && random < 3) {
    Cashiers[my_line]->addToBribeLine(this);
    std::cout << this->name << " has gotten in bribe line for " << Cashiers[my_line]->getName() << std::endl;
    Cashiers[my_line]->getBribeLineCV()->Wait(CashierLineLock);
    money -= 500;
    if (test6) MONEY += 500;
  } else {
    if (Cashiers[my_line]->getState() == 1 || Cashiers[my_line]->getState() == 2) {
      Cashiers[my_line]->addToLine(this);
      std::cout << this->name << " has gotten in regular line for " << Cashiers[my_line]->getName() << std::endl;
      Cashiers[my_line]->getLineCV()->Wait(CashierLineLock);
    } else {
      Cashiers[my_line]->setCurrentCustomer(this);
    }
  }

  Cashiers[my_line]->setState(1);
  CashierLineLock->Release();

  return my_line;
}

void Customer::PayCashier(int my_line) {
    Cashiers[my_line]->Acquire();

    // Give SSN
    Cashiers[my_line]->getCV()->Signal(Cashiers[my_line]->getLock());
    std::cout << this->name << " has given SSN " << ssn << " to " << Cashiers[my_line]->getName() << std::endl;

    // Wait to determine whether they go back in line
    Cashiers[my_line]->getCV()->Wait(Cashiers[my_line]->getLock());

    if(sendToBackOfLine){
      //send customer to back of line after yield
      std::cout << this->name << " has gone to " << Cashiers[my_line]->getName() << " too soon. ";
      std::cout << "They are going to the back of the line." << std::endl;

      // Signal cashier that I'm leaving
      Cashiers[my_line]->getCV()->Signal(Cashiers[my_line]->getLock());

      // Punishment
      Cashiers[my_line]->Release(); // no busy waiting
      for(int i =100; i<1000; ++i){
          currentThread->Yield();
      }

      sendToBackOfLine = false;
      PayCashier(FindCashierLine());
      return;
    }
    
    Cashiers[my_line]->getCV()->Signal(Cashiers[my_line]->getLock());
    money -= 100;
    if (test6) MONEY += 100;
    std::cout << this->name << " has given " << Cashiers[my_line]->getName() << " $100" << std::endl;

    //waits for cashier to give me passport
    Cashiers[my_line]->getCV()->Wait(Cashiers[my_line]->getLock());

    //ensure cashier that i've been given my passport
    Cashiers[my_line]->getCV()->Signal(Cashiers[my_line]->getLock());

    Cashiers[my_line]->getCV()->Wait(Cashiers[my_line]->getLock());

    // Signal cashier that I'm leaving
    Cashiers[my_line]->getCV()->Signal(Cashiers[my_line]->getLock());

    Cashiers[my_line]->Release();  
}

void CustomerStart(int index) {
  Customers[index]->CustomerStart();
}

void AppClerkStart(int index) {
  AppClerks[index]->AppClerkStart();
}

void PicClerkStart(int index){
  PicClerks[index]->PicClerkStart();
}

void PassportClerkStart(int index){
  PassportClerks[index]->PassportClerkStart();
}

void CashierStart(int index) {
  Cashiers[index]->CashierStart();
}

void ManagerStart() {
  manager->ManagerStart();
}

void CustomerTest1(int index) {
  Customers[index]->GetApplicationFiled(Customers[index]->FindAppLine());
}

void CustomerTest3(int index) {
  Customers[index]->PayCashier(Customers[index]->FindCashierLine());
  std::cout << Customers[index]->getName() << " is leaving the Passport Office" << std::endl;
  sem.V();
}

void CustomerTest5(int index) {
  Customers[index]->GetApplicationFiled(Customers[index]->FindAppLine());
  sem.V();
}

void TEST_1() {
  /* Customers always take the shortest line, but no 2 customers 
  ever choose the same shortest line at the same time */
  NUM_CUSTOMERS = 2;
  NUM_APP_CLERKS = 2;
  int SSN = 100000000;

  std::cout << "Number of Customers = " << NUM_CUSTOMERS << std::endl;
  std::cout << "Number of ApplicationClerks = " << NUM_APP_CLERKS << std::endl;
  std::cout << "Number of PictureClerks = 0" << std::endl;
  std::cout << "Number of PassportClerks = 0" << std::endl;
  std::cout << "Number of Cashiers = 0" << std::endl;
  std::cout << "Number of Senators = 0" << std::endl;
  std::cout << std::endl;
  std::cout << "This test will only run for the ApplicationClerk." << std::endl;
  std::cout << "ApplicationClerk 0 will start with 2 customers, simulating a 'longer' line" << std::endl;
  std::cout << std::endl;

  Customers = new Customer*[NUM_CUSTOMERS];
  AppClerks = new AppClerk*[NUM_APP_CLERKS];

  AppClerkLineLock = new Lock("appClerk_lineLock");

  for(int i = 0; i < NUM_APP_CLERKS; i++) {
    char* debugName = new char[20];
    sprintf(debugName, "ApplicationClerk %d", i);
    AppClerks[i] = new AppClerk(debugName, i);
  }

  for(int i = 0; i < NUM_CUSTOMERS; i++){
    char* debugName = new char[20];
    SSN += rand()%(90000000/NUM_CUSTOMERS);
    sprintf(debugName, "Customer %d", i);
    Customers[i] = new Customer(debugName, SSN);
  }

  AppClerks[0]->addToLine(new Customer("dummy", 0));
  AppClerks[0]->addToLine(new Customer("dummy", 1));

  std::cout << "TEST_1: ApplicationClerk 0 has " << AppClerks[0]->getLineSize() << "Customers" << std::endl;
  std::cout << "TEST_1: ApplicationClerk 1 has " << AppClerks[1]->getLineSize() << "Customers" << std::endl;


  AppClerks[1]->Fork((VoidFunctionPtr)AppClerkStart, 1);

  for(int i = 0; i < NUM_CUSTOMERS; i++){
    Customers[i]->Fork((VoidFunctionPtr)CustomerTest1, i);
  }
}

void TEST_2() {
  // Managers only read one from one Clerk's total money received, at a time
}

void TEST_3() {
  /* Customers do not leave until they are given their passport by the Cashier. 
  The Cashier does not start on another customer until they know that the last 
  Customer has left their area */

  NUM_CUSTOMERS = 3;
  NUM_CASHIERS = 1;
  int SSN = 100000000;

  std::cout << "Number of Customers = " << NUM_CUSTOMERS << std::endl;
  std::cout << "Number of ApplicationClerks = 0" << std::endl;
  std::cout << "Number of PictureClerks = 0" << std::endl;
  std::cout << "Number of PassportClerks = 0" << std::endl;
  std::cout << "Number of Cashiers = " << NUM_CASHIERS << std::endl;
  std::cout << "Number of Senators = 0" << std::endl;
  std::cout << std::endl;
  std::cout << "This test will only run for the Cashier." << std::endl;
  std::cout << std::endl;

  Customers = new Customer*[NUM_CUSTOMERS];
  Cashiers = new Cashier*[NUM_CASHIERS];
  manager = new Manager("Manager", 0);

  CashierLineLock = new Lock("cashier_lineLock");

  char* debugName = new char[20];
  sprintf(debugName, "Cashier %d", 0);
  Cashiers[0] = new Cashier(debugName, 0);

  for(int i = 0; i < NUM_CUSTOMERS; i++){
    debugName = new char[20];
    SSN += rand()%(90000000/NUM_CUSTOMERS);
    sprintf(debugName, "Customer %d", i);
    Customers[i] = new Customer(debugName, SSN);
  }

  Cashiers[0]->Fork((VoidFunctionPtr)CashierStart, 0);
  manager->Fork((VoidFunctionPtr)ManagerStart, 0);

  for(int i = 0; i < NUM_CUSTOMERS; i++){
    Customers[i]->Fork((VoidFunctionPtr)CustomerTest3, i);
  }
}

void TEST_4() {
    // Clerks go on break when they have no one waiting in their line
    // Managers only read one from one Clerk's total money received, at a time

  NUM_CUSTOMERS = 0;
  NUM_APP_CLERKS = 2;

  AppClerks = new AppClerk*[NUM_APP_CLERKS];

  AppClerkLineLock = new Lock("appClerk_lineLock");

  for(int i = 0; i < NUM_APP_CLERKS; i++) {
    char* debugName = new char[15];
    sprintf(debugName, "appClerk_%d", i);
    AppClerks[i] = new AppClerk(debugName, i);
  }

  for(int i = 0; i < NUM_APP_CLERKS; i++) {
    AppClerks[i]->Fork((VoidFunctionPtr)AppClerkStart, i);
  }
}

void TEST_5() {
  // Managers get Clerks off their break when lines get too long

  NUM_CUSTOMERS = 4;
  NUM_APP_CLERKS = 1;

  Customers = new Customer*[NUM_CUSTOMERS];
  AppClerks = new AppClerk*[NUM_APP_CLERKS];
  AppClerkLineLock = new Lock("appClerk_lineLock");


  for(int i = 0; i < NUM_APP_CLERKS; i++) {
    char* debugName = new char[15];
    sprintf(debugName, "ApplicationClerk %d", i);
    AppClerks[i] = new AppClerk(debugName, i);
  }

  AppClerks[0] -> setState(2);

  for(int i = 0; i < NUM_CUSTOMERS; i++){
    char* debugName = new char[15];
    sprintf(debugName, "Customer %d", i);
    Customers[i] = new Customer(debugName, i);
  }

  manager = new Manager("Manager", 0);

  for(int i = 0; i < NUM_APP_CLERKS; i++) {
    AppClerks[i]->Fork((VoidFunctionPtr)AppClerkStart, i);
  }

  manager->Fork((VoidFunctionPtr)ManagerStart, 0);

  for(int i = 0; i < NUM_CUSTOMERS; i++){
    Customers[i]->Fork((VoidFunctionPtr)CustomerTest5, i);
  }
}

void TEST_6() {
  // Total sales never suffers from a race condition
  MONEY = 0;
  NUM_CUSTOMERS = 12;
  NUM_APP_CLERKS = 2;
  NUM_PIC_CLERKS = 2;
  NUM_PASSPORT_CLERKS = 2;
  NUM_CASHIERS = 2;
  Customers = new Customer*[NUM_CUSTOMERS];
  AppClerks = new AppClerk*[NUM_APP_CLERKS];
  PicClerks = new PicClerk*[NUM_PIC_CLERKS];
  PassportClerks = new PassportClerk*[NUM_PASSPORT_CLERKS];
  Cashiers = new Cashier*[NUM_CASHIERS];
  manager = new Manager("Manager", 0);
  int SSN = 10000000;

  std::cout << "Number of Customers = " << NUM_CUSTOMERS << std::endl;
  std::cout << "Number of ApplicationClerks = " << NUM_APP_CLERKS << std::endl;
  std::cout << "Number of PictureClerks = " << NUM_PIC_CLERKS << std::endl;
  std::cout << "Number of PassportClerks = " << NUM_PASSPORT_CLERKS << std::endl;
  std::cout << "Number of Cashiers = " << NUM_CASHIERS << std::endl;
  std::cout << "Number of Senators = 0" << std::endl;
  std::cout << std::endl;

  AppClerkLineLock = new Lock("appClerk_lineLock");
  PicClerkLineLock = new Lock("picClerk_lineLock");
  PassportClerkLineLock = new Lock("passportClerk_lineLock");
  CashierLineLock = new Lock("cashier_lineLock");

  for(int i = 0; i < NUM_APP_CLERKS; i++) {
    char* debugName = new char[20];
    sprintf(debugName, "ApplicationClerk %d", i);
    AppClerks[i] = new AppClerk(debugName, i);
  }

  for(int i = 0; i < NUM_PIC_CLERKS; i++) {
    char* debugName = new char[20];
    sprintf(debugName, "PictureClerk %d", i);
    PicClerks[i] = new PicClerk(debugName, i);
  }

  for(int i = 0; i < NUM_PASSPORT_CLERKS; i++) {
    char* debugName = new char[20];
    sprintf(debugName, "PassportClerk %d", i);
    PassportClerks[i] = new PassportClerk(debugName, i);
  }

  for(int i = 0; i < NUM_CASHIERS; i++) {
    char* debugName = new char[20];
    sprintf(debugName, "Cashier %d", i);
    Cashiers[i] = new Cashier(debugName, i);
  }

  for(int i = 0; i < NUM_CUSTOMERS; i++){
    char* debugName = new char[15];
    SSN += rand()%(90000000/NUM_CUSTOMERS);
    sprintf(debugName, "Customer %d", i);
    Customers[i] = new Customer(debugName, SSN);
  }

  for(int i = 0; i < NUM_APP_CLERKS; i++) {
    AppClerks[i]->Fork((VoidFunctionPtr)AppClerkStart, i);
  }

  for(int i = 0; i < NUM_PIC_CLERKS; i++) {
    PicClerks[i]->Fork((VoidFunctionPtr)PicClerkStart, i);
  }

  for(int i = 0; i < NUM_PASSPORT_CLERKS; i++) {
    PassportClerks[i]->Fork((VoidFunctionPtr)PassportClerkStart, i);
  }

  for(int i = 0; i < NUM_CASHIERS; i++) {
    Cashiers[i]->Fork((VoidFunctionPtr)CashierStart, i);
  }

  manager->Fork((VoidFunctionPtr)ManagerStart, 0);

  for(int i = 0; i < NUM_CUSTOMERS; i++){
    Customers[i]->Fork((VoidFunctionPtr)CustomerStart, i);
  }
}

void TEST_7() {
  /* The behavior of Customers is proper when Senators arrive. This is
  before, during, and after. */
}

void FULL_SIMULATION() {
  std::cout << "Number of Customers = " << NUM_CUSTOMERS << std::endl;
  std::cout << "Number of ApplicationClerks = " << NUM_APP_CLERKS << std::endl;
  std::cout << "Number of PictureClerks = " << NUM_PIC_CLERKS << std::endl;
  std::cout << "Number of PassportClerks = " << NUM_PASSPORT_CLERKS << std::endl;
  std::cout << "Number of Cashiers = " << NUM_CASHIERS << std::endl;
  std::cout << "Number of Senators = 0" << std::endl;
  std::cout << std::endl;

  Customers = new Customer*[NUM_CUSTOMERS];
  AppClerks = new AppClerk*[NUM_APP_CLERKS];
  PicClerks = new PicClerk*[NUM_PIC_CLERKS];
  PassportClerks = new PassportClerk*[NUM_PASSPORT_CLERKS];
  Cashiers = new Cashier*[NUM_CASHIERS];
  int SSN = 10000000;

  AppClerkLineLock = new Lock("appClerk_lineLock");
  PicClerkLineLock = new Lock("picClerk_lineLock");
  PassportClerkLineLock = new Lock("passportClerk_lineLock");
  CashierLineLock = new Lock("cashier_lineLock");

  for(int i = 0; i < NUM_APP_CLERKS; i++) {
    char* debugName = new char[20];
    sprintf(debugName, "ApplicationClerk %d", i);
    AppClerks[i] = new AppClerk(debugName, i);
  }

  for(int i = 0; i < NUM_PIC_CLERKS; i++) {
    char* debugName = new char[20];
    sprintf(debugName, "PictureClerk %d", i);
    PicClerks[i] = new PicClerk(debugName, i);
  }

  for(int i = 0; i < NUM_PASSPORT_CLERKS; i++) {
    char* debugName = new char[20];
    sprintf(debugName, "PassportClerk %d", i);
    PassportClerks[i] = new PassportClerk(debugName, i);
  }

  for(int i = 0; i < NUM_CASHIERS; i++) {
    char* debugName = new char[20];
    sprintf(debugName, "Cashier %d", i);
    Cashiers[i] = new Cashier(debugName, i);
  }

  for(int i = 0; i < NUM_CUSTOMERS; i++){
    char* debugName = new char[15];
    SSN += rand()%(90000000/NUM_CUSTOMERS);
    sprintf(debugName, "Customer %d", i);
    Customers[i] = new Customer(debugName, SSN);
  }

  for(int i = 0; i < NUM_APP_CLERKS; i++) {
    AppClerks[i]->Fork((VoidFunctionPtr)AppClerkStart, i);
  }

  for(int i = 0; i < NUM_PIC_CLERKS; i++) {
    PicClerks[i]->Fork((VoidFunctionPtr)PicClerkStart, i);
  }

  for(int i = 0; i < NUM_PASSPORT_CLERKS; i++) {
    PassportClerks[i]->Fork((VoidFunctionPtr)PassportClerkStart, i);
  }

  for(int i = 0; i < NUM_CASHIERS; i++) {
    Cashiers[i]->Fork((VoidFunctionPtr)CashierStart, i);
  }

  for(int i = 0; i < NUM_CUSTOMERS; i++){
    Customers[i]->Fork((VoidFunctionPtr)CustomerStart, i);
  }
}

void Problem2() {
    // Tests we have to write for problem 2
  Thread *t;
  AppClerkBribeMoney = 0;
  PicClerkBribeMoney = 0;
  PassportClerkBribeMoney = 0;
  CashierMoney = 0;
  test1 = false;
  test6 = false;

  std::cout << "Please select which test you would like to run:" << std::endl;
  std::cout << " 1. Customers always take the shortest line, but no 2 customers ever choose the same shortest line at the same time" << std::endl;
  std::cout << " 2. Managers only read one from one Clerk's total money received, at a time." << std::endl;
  std::cout << " 3. Customers do not leave until they are given their passport by the Cashier. The Cashier does not start on another customer until they know that the last Customer has left their area" << std::endl;
  std::cout << " 4. Clerks go on break when they have no one waiting in their line" << std::endl;
  std::cout << " 5. Managers get Clerks off their break when lines get too long" << std::endl;
  std::cout << " 6. Total sales never suffers from a race condition" << std::endl;
  std::cout << " 7. The behavior of Customers is proper when Senators arrive. This is before, during, and after." << std::endl;
  std::cout << " 8. Full simulation" << std::endl;
  std::cout << " 9. Quit" << std::endl;

  int testSelection = 0;
  while(testSelection != 9) {
    std::cout << "Enter option: ";
    std::cin >> testSelection;
    if(testSelection == 1) {
      test1 = true;
      std::cout << "-- Starting Test 1"<<std::endl;
      t = new Thread("ts2_t1");
      t->Fork((VoidFunctionPtr)TEST_1,0);
      for (int i = 0; i < 2; i++) {
        sem.P();
      }
      std::cout << "-- Test 1 Completed" << std::endl;
      test1 = false;
    }
    else if(testSelection == 2) {
      std::cout << "-- Starting Test 2" << std::endl;
      t = new Thread("ts2_t2");
      //t->Fork((VoidFunctionPtr)TEST_2, 0);
      std::cout << "-- Test 2 Completed" << std::endl;
    }
    else if(testSelection == 3) {
      std::cout << "-- Starting Test 3" << std::endl;
      t = new Thread("ts2_t3");
      t->Fork((VoidFunctionPtr)TEST_3, 0);
      for (int i = 0; i < 3; i++) {
        sem.P();
      }
      std::cout << "-- Test 3 Completed" << std::endl;
    }
    else if(testSelection == 4) {
      std::cout << "-- Starting Test 4" << std::endl;
      t = new Thread("ts2_t4");
      t->Fork((VoidFunctionPtr)TEST_4, 0);
      sem.P();
      std::cout << "-- Test 4 Completed" << std::endl;
    }
    else if(testSelection == 5) {
      std::cout << "-- Starting Test 5" << std::endl;
      t = new Thread("ts2_t5");
      t->Fork((VoidFunctionPtr)TEST_5, 0);
      for (int i = 0; i < 4; i++){
        sem.P();
      }
      std::cout << "-- Test 5 Completed" << std::endl;
    }
    else if(testSelection == 6) {
      std::cout << "-- Starting Test 6" << std::endl;
      test6 = true;
      t = new Thread("ts2_t6");
      t->Fork((VoidFunctionPtr)TEST_6, 0);
      for (int i = 0; i < 12; i++) {
        sem.P();
      }
      if (MONEY == (AppClerkBribeMoney + PicClerkBribeMoney + 
        PassportClerkBribeMoney + CashierMoney)) { 
        std::cout << "Test 6 PASSED" << std::endl;
      } else {
        std::cout << "Test 6 FAILED" << std::endl;
      }
      std::cout << "-- Test 6 Completed" << std::endl;
      test6 = false;
    }
    else if(testSelection == 7) {
      std::cout << "-- Starting Test 7" << std::endl;
      t = new Thread("ts2_t7");
      //t->Fork((VoidFunctionPtr)TEST_7, 0);
      std::cout << "-- Test 7 Completed" << std::endl;
    }
    else if(testSelection == 8) {
      std::cout << "-- Starting Full Simulation" << std::endl;
      std::cout << "How many customers? ";
      std::cin >> NUM_CUSTOMERS;
      std::cout << "How many application clerks? ";
      std::cin >> NUM_APP_CLERKS;
      std::cout << "How many picture clerks? ";
      std::cin >> NUM_PIC_CLERKS;
      std::cout << "How many passport clerks? ";
      std::cin >> NUM_PASSPORT_CLERKS;
      std::cout << "How many cashiers? ";
      std::cin >> NUM_CASHIERS;

      // make sure to print out how many for each var
      t = new Thread("ts2_fullsimulation");
      t->Fork((VoidFunctionPtr)FULL_SIMULATION, 0);
      for (int i = 0; i < NUM_CUSTOMERS; i++) {
        sem.P();
      }
      std::cout << "-- Full Simulation Completed" << std::endl;
    }
    else if(testSelection == 9) {    
      std::cout << "Quitting!" << std::endl;
      return;
    }
    else {
      printf("-- not a valid choice, please try again --");
    }
  }
}

