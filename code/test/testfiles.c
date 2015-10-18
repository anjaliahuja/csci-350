/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

int t1_l1, t1_l2, t1_l3, t2_l1, t3_l1, t4_l1, t5_l1, t5_l2;
int t2_c1, t2_c2, t2_c3, t3_c1, t4_c1, t5_c1;

<<<<<<< HEAD

void t1_t4();
void t1_t5();
=======
>>>>>>> master
void startTest2();
void t2_t4();
void startTest3();
void startTest4();
void startTest5();

/* 
   --------------------------------------------------
   t1_t1() -- test1 thread 1
       This thread will acquire and release lock
   --------------------------------------------------
*/
void t1_t1() {
  int i;
  Write("In t1_t1\n", sizeof("In t1_t1\n"), ConsoleOutput);


  Acquire(t1_l1);
  Write("t1_t1 acquired t1_l1\n", sizeof("t1_t1 acquired t1_l1\n"), ConsoleOutput);

  Write("t1_t1 releasing t1_l1\n", sizeof("t1_t1 releasing t1_l1\n"), ConsoleOutput);
  Release(t1_l1);

  
  Exit(0);
}

/* 
   --------------------------------------------------
   t1_t2() -- test1 thread 2
       This thread will try to release the lock illegally
   --------------------------------------------------
*/
void t1_t2() {
  Write("t1_t2 releasing t1_l1\n", sizeof("t1_t2 releasing t1_l1\n"), ConsoleOutput);
  Release(t1_l1);

  Exit(0);
}

/* 
   --------------------------------------------------
   t1_t3() -- test1 thread 3
       This thread will try to acquire the lock after it's been destroyed.
   --------------------------------------------------
*/
void t1_t3() {
  Write("t1_t3 destroying t1_l1\n", sizeof("t1_t3 destroying t1_l1\n"), ConsoleOutput);
  DestroyLock(t1_l1);

  Write("t1_t3 acquiring t1_l1\n", sizeof("t1_t3 acquiring t1_l1\n"), ConsoleOutput);
  Acquire(t1_l1);

  Exit(0);
}

/* 
   --------------------------------------------------
   t1_t4() -- test1 thread 4
       This thread will try to destroy a busy lock.  
       After the lock is released, the thread will try to acquire it again but
       it will have been destroyed.
       Will also show that the thread can't acquire the lock twice.
   --------------------------------------------------
*/
void t1_t4() {
  t1_l1 = CreateLock("t1_l1", 5);
  Write("t1_t4 acquiring t1_l1\n", sizeof("t1_t4 acquiring t1_l1\n"), ConsoleOutput);
  Acquire(t1_l1);

  Write("t1_t4 destroying t1_l1\n", sizeof("t1_t3 destroying t1_l1\n"), ConsoleOutput);
  DestroyLock(t1_l1);  

  Write("t1_t4 acquiring t1_l1 again to show it still exists\n", sizeof("t1_t4 acquiring t1_l1 again to show it still exists\n"), ConsoleOutput);
  Acquire(t1_l1);

  Write("t1_t4 releasing t1_l1\n", sizeof("t1_t4 releasing t1_l1\n"), ConsoleOutput);
  Release(t1_l1);

  Write("t1_t4 acquiring t1_l1 after it was destroyed\n", sizeof("t1_t4 acquiring t1_l1 after it was destroyed\n"), ConsoleOutput);
  Acquire(t1_l1);

  Exit(0);
}

/* 
   --------------------------------------------------
   t1_t5() -- test1 thread 5
       Creating multiple locks and Accessing invalid indices.
   --------------------------------------------------
*/
void t1_t5() {
  Write("t1_t5 creating tl_l2 and tl_l3\n", sizeof("t1_t5 creating tl_l2 and tl_l3\n"), ConsoleOutput);
  t1_l2 = CreateLock("t1_l2", 5);
  t1_l3 = CreateLock("t1_l3", 5);

  Write("t1_t5 acquiring tl_l2 and tl_l3\n", sizeof("t1_t5 acquiring tl_l2 and tl_l3\n"), ConsoleOutput);

  Acquire(t1_l2);
  Acquire(t1_l3);

  Write("t1_t5 acquiring and releasing at index -1\n", sizeof("t1_t5 acquiring and releasing at index -1\n"), ConsoleOutput);
  Acquire(-1);
  Release(-1);

  Exit(0);
}

/* 
   --------------------------------------------------
   t2_t1() -- test2 thread 1
       This thread will signal a variable with nothing waiting
   --------------------------------------------------
*/
void t2_t1() {

  Acquire(t2_l1);
  Write("t2_t1 acquired t2_l1, signalling t2_c1\n", sizeof("t2_t1 acquired t2_l1, signalling t2_c1\n"), ConsoleOutput);

  Signal(t2_l1, t2_c1);

  Write("t2_t1 releasing t2_l1\n", sizeof("t2_t1 releasing t2_l1\n"), ConsoleOutput);
  Release(t2_l1);

  Exit(0);
}

/* 
   --------------------------------------------------
   t2_t2() -- test2 thread 2
       This thread will wait on a pre-signalled variable
   --------------------------------------------------
*/
void t2_t2() {
  Acquire(t2_l1);

  Write("t2_t2 acquired t2_l1, waiting on t2_c1\n", sizeof("t2_t2 acquired t2_l1, waiting on t2_c1\n"), ConsoleOutput);
  
  Wait(t2_l1, t2_c1);

  Write("t2_t2 releasing t2_l1\n", sizeof("t2_t2 releasing t2_l1\n"), ConsoleOutput);
  Release(t2_l1);

  Exit(0);
}

/* 
   --------------------------------------------------
   t2_t3() -- test2 thread 3
       This thread will signal the waiting thread from t2_t1.
   --------------------------------------------------
*/
void t2_t3() {
  Write("t2_t3 acquiring t2_l1\n", sizeof("t2_t3 acquiring t2_l1\n"), ConsoleOutput);
  Acquire(t2_l1);

  Write("t2_t3 signalling t2_c1\n", sizeof("t2_t3 signalling t2_c1\n"), ConsoleOutput);  
  Signal(t2_l1, t2_c1);

  Write("t2_t3 releasing t2_l1\n", sizeof("t2_t3 releasing t2_l1\n"), ConsoleOutput);
  Release(t2_l1);

  Fork(t2_t4, "t2_t4", sizeof(t2_t4));

  Exit(0);
}
/* 
   --------------------------------------------------
   t2_t4() -- test2 thread 4
       This thread will destroy t2_c2 and then try to signal it.
   --------------------------------------------------
*/
void t2_t4() {
  Write("t2_t4 acquiring t2_l1\n", sizeof("t2_t4 acquiring t2_l1\n"), ConsoleOutput);
  Acquire(t2_l1);

  Write("t2_t4 destroying t2_c1\n", sizeof("t2_t4 destroying t2_c1\n"), ConsoleOutput);
  DestroyCV(t2_c1);

  Write("t2_t4 signalling destroyed t2_c1\n", sizeof("t2_t4 signalling destroyed t2_c1\n"), ConsoleOutput);  
  Signal(t2_l1, t2_c1);

  Write("t2_t4 releasing t2_l1\n", sizeof("t2_t4 releasing t2_l1\n"), ConsoleOutput);
  Release(t2_l1);

  Exit(0);
}

/* 
   --------------------------------------------------
   t2_t5() -- test2 thread 5
       Creating multiple cvs and Accessing invalid indices.
   --------------------------------------------------
*/
void t2_t5() {
  Write("t2_t5 creating t5_c2 and t5_c3\n", sizeof("t2_t5 creating t5_c2 and t5_c3\n"), ConsoleOutput);
  t2_c2 = CreateCV("t2_l2", 5);
  t2_c3 = CreateCV("t2_l3", 5);

  Write("t2_t5 waiting, signalling, and broadcasting at index -1\n", sizeof("t2_t5 waiting, signalling, and broadcasting at index -1\n"), ConsoleOutput);
  Wait(-1, -1);
  Signal(-1, -1);
  Broadcast(-1, -1);

  Exit(0);
}

void startTest2() {
  Write("\nTest 2\n", sizeof("\nTest 2\n"), ConsoleOutput);

  t2_l1 = CreateLock("t2_l1", 5); 
  t2_c1 = CreateCV("t2_c1", 5);

  Fork(t2_t1, "t2_t1", sizeof("t2_t1"));
  Fork(t2_t2, "t2_t2", sizeof("t2_t1"));
  Fork(t2_t3, "t2_t3", sizeof("t2_t3"));
  Fork(t2_t5, "t2_t5", sizeof("t2_t5"));

  startTest3();
}

/* --------------------------------------------------
// t3_waiter()
//     These threads will wait on the t3_c1 condition variable.  Only
//     one t3_waiter will be released
// --------------------------------------------------
*/
void t3_waiter() {
    Acquire(t3_l1);
    Write("t3_waiter acquired t3_l1, waiting on t3_c1\n", sizeof("t3_waiter acquired t3_l1, waiting on t3_c1\n"), ConsoleOutput);
    
    Wait(t3_l1, t3_c1);

    Write("t3_waiter freed from t3_c1\n", sizeof("t3_waiter freed from t3_c1\n"), ConsoleOutput);

    Release(t3_l1);

    Exit(0);
}


/* --------------------------------------------------
// t3_signaller()
//     This threads will signal the t3_c1 condition variable.  Only
//     one t3_signaller will be released
// --------------------------------------------------
*/
void t3_signaller() {
    Acquire(t3_l1);

    Write("t3_signaller acquired t3_l1, signalling t3_c1\n", sizeof("t3_signaller acquired t3_l1, signalling t3_c1\n"), ConsoleOutput);

    Signal(t3_l1, t3_c1);

    Write("t3_signaller releasing t3_l1\n", sizeof("t3_signaller releasing t3_l1\n"), ConsoleOutput);

    Release(t3_l1);

    Exit(0);
}

void startTest3() {
  int i;
  Write("\nTest 3\n", sizeof("\nTest 3\n"), ConsoleOutput);

  t3_l1 = CreateLock("t3_l1", sizeof("t3_l1"));
  t3_c1 = CreateCV("t3_c1", sizeof("t3_c1"));

  /* Need to find a way to fork threads in a for loop 
    so that each thread has a unique name.
  */
  for(i = 0; i < 5; i++) {
    Fork(t3_waiter, "t3_waiter", sizeof("t3_waiter"));
  }

  Fork(t3_signaller, "t3_signaller", sizeof("t3_signaller"));

  startTest4();
}
/* --------------------------------------------------
// t4_waiter()
//     These threads will wait on the t4_c1 condition variable.  All
//     t4_waiters will be released
// --------------------------------------------------
*/
void t4_waiter() {
    Acquire(t4_l1);
    Write("t4_waiter acquired t4_l1, waiting on t4_c1\n", sizeof("t4_waiter acquired t4_l1, waiting on t4_c1\n"), ConsoleOutput);

    Wait(t4_l1, t4_c1);

    Write("t4_waiter freed from t4_c1\n", sizeof("t4_waiter freed from t4_c1\n"), ConsoleOutput);
    Release(t4_l1);

    Exit(0);
}


/* --------------------------------------------------
// t2_signaller()
//     This thread will broadcast to the t4_c1 condition variable.
//     All t4_waiters will be released
// --------------------------------------------------
*/
void t4_signaller() {
    Acquire(t4_l1);
    Write("t4_signaller acquired t4_l1, broadcasting t4_c1\n", sizeof("t4_signaller acquired t4_l1, broadcasting t4_c1\n"), ConsoleOutput);

    Broadcast(t4_l1, t4_c1);

    Write("t4_signaller releasing t4_l1\n", sizeof("t4_signaller releasing t4_l1\n"), ConsoleOutput);
    Release(t4_l1);

    Exit(0);
}

void startTest4() {
  int i;
  Write("\nTest 4\n", sizeof("\nTest 4\n"), ConsoleOutput);

  t4_l1 = CreateLock("t4_l1", sizeof("t4_l1"));
  t4_c1 = CreateCV("t4_c1", sizeof("t4_c1"));

  /* Need to find a way to fork threads in a for loop 
    so that each thread has a unique name.
  */
  for(i = 0; i < 5; i++) {
    Fork(t4_waiter, "t4_waiter", sizeof("t4_waiter"));
  }

  Fork(t4_signaller, "t4_signaller", sizeof("t4_signaller"));

  startTest5();
}
/*
// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a condition under t5_l1
// --------------------------------------------------
*/
void t5_t1() {
  Acquire(t5_l1);
  Write("t5_t1 acquired t5_l1, waiting on t5_c1\n", sizeof("t5_t1 acquired t5_l1, waiting on t5_c1\n"), ConsoleOutput);
  
  Wait(t5_l1, t5_c1);

  Write("t5_t1 releasing t5_l1\n", sizeof("t5_t1 releasing t5_l1\n"), ConsoleOutput);
  Release(t5_l1);
  Exit(0);
}

/*
// --------------------------------------------------
// t5_t2() -- test 5 thread 2
//     This thread will wait on a t5_c1 condition under t5_l2, which is
//     a Fatal error
// --------------------------------------------------
*/
void t5_t2() {
  Acquire(t5_l1);
  Acquire(t5_l2);
  Write("t5_t2 acquired t5_l2, signalling t5_c1\n", sizeof("t5_t2 acquired t5_l2, signalling t5_c1\n"), ConsoleOutput);

  Signal(t5_l2, t5_c1);

  Write("t5_t2 releasing t5_l2\n", sizeof("t5_t2 releasing t5_l2\n"), ConsoleOutput);
  Release(t5_l2);
  Write("t5_t2 releasing t5_l1\n", sizeof("t5_t2 releasing t5_l1\n"), ConsoleOutput);
  Release(t5_l1);
  Exit(0);
}

void startTest5() {
  Write("\nTest 5\n", sizeof("\nTest 5\n"), ConsoleOutput);
  t5_l1 = CreateLock("t5_l1", sizeof("t5_l1"));
  t5_l2 = CreateLock("t5_l2", sizeof("t5_l2"));
  t5_c1 = CreateCV("t5_c1", sizeof("t5_c1"));

  Fork(t5_t1, "t5_t1", sizeof("t5_t1"));
  Fork(t5_t2, "t5_t2", sizeof("t5_t2"));
}


int main() {
  int i;
  OpenFileId fd;
  int bytesread;
  char buf[20];

  Create("testfile", 8);
  fd = Open("testfile", 8);

  Write("testing a write\n", 16, fd );
  Close(fd);


  fd = Open("testfile", 8);
  bytesread = Read( buf, 100, fd );
  Write( buf, bytesread, ConsoleOutput );
  Close(fd);

  t1_l1 = CreateLock("t1_l1", 5); 

  Write("Test 1\n", sizeof("Test 1\n"), ConsoleOutput);

  Fork(t1_t1, "t1_t1", sizeof("t1_t1"));
  Fork(t1_t2, "t1_t2", sizeof("t1_t2"));
  Fork(t1_t3, "t1_t3", sizeof("t1_t3"));
  Fork(t1_t4, "t1_t4", sizeof("t1_t4"));
  Fork(t1_t5, "t1_t5", sizeof("t1_t5"));

  startTest2();
}
