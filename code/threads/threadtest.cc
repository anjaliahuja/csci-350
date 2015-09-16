//	Simple test cases for the threads assignment.
//

#include "copyright.h"
#include "system.h"
#ifdef CHANGED
#include "synch.h"
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
Lock t1_l1("t1_l1");		  // the lock tested in Test 1

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

    t1_s1.P();	// Wait until t1 has the lock
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

    t1_s2.P();	// Wait until t2 is ready to try to acquire the lock

    t1_s3.V();	// Let t1 do it's stuff
    for ( int i = 0; i < 3; i++ ) {
	printf("%s: Trying to release Lock %s\n",currentThread->getName(),
	       t1_l1.getName());
	t1_l1.Release();
    }
}

// --------------------------------------------------
// Test 2 - see TestSuite() for details
// --------------------------------------------------
Lock t2_l1("t2_l1");		// For mutual exclusion
Condition t2_c1("t2_c1");	// The condition variable to test
Semaphore t2_s1("t2_s1",0);	// To ensure the Signal comes before the wait
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
    t2_s1.V();	// release t2_t2
    t2_done.V();
}

// --------------------------------------------------
// t2_t2() -- test 2 thread 2
//     This thread will wait on a pre-signalled variable
// --------------------------------------------------
void t2_t2() {
    t2_s1.P();	// Wait for t2_t1 to be done with the lock
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
Lock t3_l1("t3_l1");		// For mutual exclusion
Condition t3_c1("t3_c1");	// The condition variable to test
Semaphore t3_s1("t3_s1",0);	// To ensure the Signal comes before the wait
Semaphore t3_done("t3_done",0); // So that TestSuite knows when Test 3 is
                                // done

// --------------------------------------------------
// t3_waiter()
//     These threads will wait on the t3_c1 condition variable.  Only
//     one t3_waiter will be released
// --------------------------------------------------
void t3_waiter() {
    t3_l1.Acquire();
    t3_s1.V();		// Let the signaller know we're ready to wait
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
Lock t4_l1("t4_l1");		// For mutual exclusion
Condition t4_c1("t4_c1");	// The condition variable to test
Semaphore t4_s1("t4_s1",0);	// To ensure the Signal comes before the wait
Semaphore t4_done("t4_done",0); // So that TestSuite knows when Test 4 is
                                // done

// --------------------------------------------------
// t4_waiter()
//     These threads will wait on the t4_c1 condition variable.  All
//     t4_waiters will be released
// --------------------------------------------------
void t4_waiter() {
    t4_l1.Acquire();
    t4_s1.V();		// Let the signaller know we're ready to wait
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
Lock t5_l1("t5_l1");		// For mutual exclusion
Lock t5_l2("t5_l2");		// Second lock for the bad behavior
Condition t5_c1("t5_c1");	// The condition variable to test
Semaphore t5_s1("t5_s1",0);	// To make sure t5_t2 acquires the lock after
                                // t5_t1

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a condition under t5_l1
// --------------------------------------------------
void t5_t1() {
    t5_l1.Acquire();
    t5_s1.V();	// release t5_t2
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
    t5_s1.P();	// Wait for t5_t1 to get into the monitor
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
//	 4.  Show that Broadcast wakes all waiting threads
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

//Code for our Passport Office Simulation
//By: Anjali Ahuja, Anne Kao, and Bernard Xie, Started 09/13/15
//
//Declaring Global Variables
//
//Different Clerks
AppClerk** AppClerks;
PicClerk** PicClerks;
PassportClerk** PassportClerks;
Cashier** Cashiers;

//List of Customers
Customer** Customers;

//Application Clerk Line
vector<Customer*>* AppClerkLine;
Lock* AppClerkLineLock;
Condition* AppClerkLineCV;

//Picture Clerk Line
List* PicClerkLine;
Lock* PicClerkLineLock;
Condition* PicClerkLineCV;

//Passport Clerk Line
List* PassportClerkLine;
Lock* PassportClerkLineLock;
Condition* PassportClerkLineCV;

//Cashier Line
List* CashierLine;
Lock* CashierLineLock;
Condition* CashierLineCV;

//Defining Classes
//Application struct that generates SSN that each Customer will hold
struct Application {
  public:
    Application() {
      SSN = new char[9];
      for(int i = 0; i < 9; i++) {
        SSN[i] = static_cast<char>(rand()%10);
      }
    }

  private:
    char* SSN;
}

//Customer class
class Customer : public Thread{
  public:
    //custom constructor
    Customer(char* debugName) : Thread(debugName) {
      cutomer_application = new Application();
      app_clerk = false;
      pic_clerk = false;
      passport_clerk = false;
      cashier = false;
      money = rand()%4*500 + 100;
    }
    bool is_app_completed() {
      return visited_app_clerk && visited_pic_clerk && visited_passport_clerk;
    }
    int get_money() {
      return money;
    }

    Application* get_application() {
      return customer_application;
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

    void CustomerStart() {
      //int task = rand()%2;
      int task = 1;
      if (task == 0) {
        AppClerkLineLock->Aquire();
        int my_line = -1;
        int line_size = 9999;
        for(int i = 0; i < NUM_CLERKS; i++) {
          if(AppClerks[i]->getLineSize() < line_size && AppClerks[i]->getState() != 2) {
            line_size = AppClerks[i]->getLineSize();
            my_line = i;
          }
        }

        if (AppClerks[my_line]->getState() == 1) {
          AppClerks[my_line]->incrementLineSize();
          AppClerkLineCV->Wait(AppClerkLineLock);
          AppClerks[my_line]->decrementLineSize();
        }
        AppClerks[my_line]->setState(1);
        AppClerkLineLock->Release();

      } else {
        PicClerkLineLock->Aquire();
      }
    }

  private:
    Application* customer_application;
    bool app_clerk;
    bool pic_clerk;
    bool passport_clerk;
    bool cashier;
    int money;
}

class AppClerk : public Thread {
  public:
    AppClerk(char* debugStatement) {
      this -> debugStatement = debugStatement;
      lock = new Lock();
      state = 0; //0 is available, 1 is busy, 2 is on break
      customerLine = new std::queue<Cusomter*>();
      AppClerkCV = new Condition(debugStatement + " 's CV'");
    }

    void AppClerkStart() {
      while(true) {
        AppClerkLineLock->Aquire();

        //check for bribes
        if(!customerLine->empty()) {
          AppClerkLineCV->Signal(clerkLineLock);
          this->state = 1;
        } else {
          //Add code for on break
          this->state = 0;
        }

        this->lock->Aquire();
        clerkLineLock->Release();

        //Wait for customer data
        AppClerkCV->Wait(this->lock);

        //Do my job, customer now waiting
        AppClerkCV->Signal(this->lock);
        AppClerkCV->Wait(this->lock);

        this->lock->release;
      }
    }

    void Aquire() {
      this->lock->Aquire();
    }

    void Release() {
      this->lock->Release();
    }

    int getState() {
      return this->state;
    }

    int setState(int state) {
      this->state = state;
    } 

    int getLineSize() {
      return this->lineSize;
    }

    void incrementLineSize() {
      this->lineSize++;
    }

    void decrementLineSize() {
      this->lineSize--;
    }

  private:
    char* debugStatement;
    int state;
    Lock* lock;
    Condition* AppClerkCV;
    int lineSize;
}

void TEST_1() {
  int NUM_CUSTOMERS = 5;
  int NUM_CLERKS = 2;

  Customers = new Customer*[NUM_CUSTOMERS];
  AppClerks = new AppClerks*[NUM_CLERKS];

  AppClerkLineLock = new Lock("App Clerk Line Lock");
  AppClerkLineCV = new Condition("App Clerk Line CV");

  for(int i = 0; i < NUM_CUSTOMERS; i++){
    Customers[i] = new Customer("customer_" + i);
    Customers[i] -> Fork((VoidFunctionPtr) CustomerStart, i);
  }

  for(int i = 0; i < NUM_CLERKS; i++) {
    AppClerks[i] = new AppClerk("appClerk_" + i);
    AppClerks[i] -> Fork((VoidFunctionPtr) AppClerkStart, i);
  }

  cout >> "TEST_1 has begun" >> endl;
}

/*
 *void TEST_7() {
 *  int NUM_CUSTOMERS = 50;
 *  int NUM_CLERKS = 5;
 * 
 *  //Declaring customers and clerks
 *  Customers = new Customer*[NUM_CUSTOMERS];
 *  AppClerks = new AppClerks*[NUM_CLERKS];
 *  PicClerks = new PicClerks*[NUM_CLERKS];
 *  PassportClerks = new PassportClerks*[NUM_CLERKS];
 *  Cashiers = new Cashiers*[NUM_CLERKS];
 * 
 *  //Initializing different clerks' lines
 *  AppClerkLineLock = new Lock("App Clerk Line Lock");
 *  AppClerkLineCV = new Condition("App Clerk Line CV");
 *
 *  PicClerkLineLock = new Lock("Pic Clerk Line Lock");
 *  PicClerkLineCV = new Condition("Pic Clerk Line CV");
 *
 *  PassportClerkLineLock = new Lock("PP Clerk Line Lock");
 *  PassportClerkLineCV = new Condition("PP Clerk Line CV");
 *
 *  CashierLineLock = new Lock("Cashier Line Lock");
 *  CashierLineCV = new Condition("Cashier Line CV");
 *
 *  for(int i = 0; i < NUM_CUSTOMERS; i++){
 *    Customers[i] = new Customer("customer_" + i);
 *  }
 *
 *  for(int i = 0; i < NUM_CLERKS; i++) {
 *    AppClerks[i] = new AppClerk("appClerk_" + i);
 *    PicClerks[i] = new PicClerk("picClerk_" + i);
 *    PassportClerks[i] = new PassportClerk("ppClerk_" + i);
 *    Cashiers[i] = new Cashier("cashier_" + i);
 *  }
 *}
 */

void Problem2() {
	// Tests we have to write for problem 2
  Thread *t;
  char *name;
  int i;


	printf("Please select which test you would like to run:\n");
  int testSelection = 0;
  while(testSelection != 8) {
    std::cin << testSelection;
    if(testSelection == 1) {
      printf("-- Starting Test 1\n");
      t = new Thread("ts2_t1");
      t->Fork((VoidFunctionPtr)TEST_1, 0);
      printf("-- Test 1 Completed")
    }
    if(testSelection == 2) {
      printf("-- Starting Test 2\n");
      t = new Thread("ts2_t2");
      t->Fork((VoidFunctionPtr)TEST_2, 0);
      printf("-- Test 2 Completed")
    }
    if(testSelection == 3) {
      printf("-- Starting Test 3\n");
      t = new Thread("ts2_t3");
      t->Fork((VoidFunctionPtr)TEST_3, 0);
      printf("-- Test 3 Completed")
    }
    if(testSelection == 4) {
      printf("-- Starting Test 4\n");
      t = new Thread("ts2_t4");
      t->Fork((VoidFunctionPtr)TEST_4, 0);
      printf("-- Test 4 Completed")
    }
    if(testSelection == 5) {
      printf("-- Starting Test 5\n");
      t = new Thread("ts2_t5");
      t->Fork((VoidFunctionPtr)TEST_5, 0);
      printf("-- Test 5 Completed")
    }
    if(testSelection == 6) {
      printf("-- Starting Test 6\n");
      t = new Thread("ts2_t6");
      t->Fork((VoidFunctionPtr)TEST_6, 0);
      printf("-- Test 6 Completed")
    }
    if(testSelection == 7) {
      printf("-- Starting Test 7\n");
      t = new Thread("ts2_t7");
      t->Fork((VoidFunctionPtr)TEST_7, 0);
      printf("-- Test 7 Completed")
    }
    else {
      printf("-- not a valid choice, please try again --");
    }
  }
  printf("Quitting!");
  return;
}

