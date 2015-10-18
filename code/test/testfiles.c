/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

int t1_l1, t1_l2, t1_l3, t2_l1, t3_l1;
int t2_c1, t3_c1;

void startTest2();

/* 
   --------------------------------------------------
   t1_t1() -- test1 thread 1
       This thread will acquire and release lock
   --------------------------------------------------
*/
void t1_t1() {
  int i;
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
/*
  Write("t1_t5 acquiring and releasing at index -1\n", sizeof("t1_t5 acquiring and releasing at index -1\n"), ConsoleOutput);
  Acquire(-1);
  Release(-1);
*/
  startTest2();
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

  Write("t2_t2 acquired t2_l1, waiting on t2_c1\n", sizeof("t2_t2 acquired t2_l1, signalling t2_c1\n"), ConsoleOutput);
  
  Wait(t2_l1, t2_c1);

  Write("t2_t2 releasing t2_l1\n", sizeof("t2_t2 releasing t2_l1\n"), ConsoleOutput);
  Release(t2_l1);

  Exit(0);
}

void startTest2() {
  Write("\nTest 2\n", sizeof("\nTest 2\n"), ConsoleOutput);

  t2_l1 = CreateLock("t2_l1", 5); 
  t2_c1 = CreateCV("t2_c1", 5);

  Fork(t2_t1, "t2_t1", sizeof("t2_t1"));
  Fork(t2_t2, "t2_t2", sizeof("t2_t1"));
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
}
