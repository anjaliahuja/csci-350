/* testfiles.c
 *  Simple program to test the file handling system calls
 */

#include "syscall.h"

int t5_l1, t5_l2;
int t5_c1;

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

int main() {
  Write("\nTest 5\n", sizeof("\nTest 5\n"), ConsoleOutput);
  t5_l1 = CreateLock("t5_l1", sizeof("t5_l1"));
  t5_l2 = CreateLock("t5_l2", sizeof("t5_l2"));
  t5_c1 = CreateCV("t5_c1", sizeof("t5_c1"));

  Fork(t5_t1, "t5_t1", sizeof("t5_t1"));
  Fork(t5_t2, "t5_t2", sizeof("t5_t2"));
}
