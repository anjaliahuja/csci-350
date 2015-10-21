/* testfiles.c
 *  Simple program to test the file handling system calls
 */

#include "syscall.h"

int t3_l1;
int t3_c1;

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

int main() {
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

}
