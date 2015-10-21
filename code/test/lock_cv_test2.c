/* testfiles.c
 *  Simple program to test the file handling system calls
 */

#include "syscall.h"

int t2_l1;
int t2_c1, t2_c2, t2_c3, t2_c4;

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

int main() {
  Write("\nTest 2\n", sizeof("\nTest 2\n"), ConsoleOutput);

  t2_l1 = CreateLock("t2_l1", 5); 
  t2_c1 = CreateCV("t2_c1", 5);

  Fork(t2_t1, "t2_t1", sizeof("t2_t1"));
  Fork(t2_t2, "t2_t2", sizeof("t2_t1"));
  Fork(t2_t3, "t2_t3", sizeof("t2_t3"));
  Fork(t2_t4, "t2_t4", sizeof("t2_t4"));
  Fork(t2_t5, "t2_t5", sizeof("t2_t5"));
}
