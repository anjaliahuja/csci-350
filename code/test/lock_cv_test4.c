/* testfiles.c
 *  Simple program to test the file handling system calls
 */

#include "syscall.h"

int t4_l1;
int t4_c1;

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

int main() {
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
}
