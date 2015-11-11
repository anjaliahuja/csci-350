/* testfiles.c
 *  Simple program to test the file handling system calls
 */

#include "syscall.h"

int i, lock;

int main() {
  lock = CreateLock("lock", 4); 

  Write("Test 1 acquiring lock\n", sizeof("Test 1 acquiring lock\n"), ConsoleOutput);
  Acquire(lock);
  Write("Test 1 acquires lock\n", sizeof("Test 1 acquires lock\n"), ConsoleOutput);

  for (i = 0; i < 50000; i++) {
    Yield();
  }

  Write("Test 1 releases lock\n", sizeof("Test 1 releases lock\n"), ConsoleOutput);
  Release(lock);
  Write("Test 1 releases lock\n", sizeof("Test 1 releases lock\n"), ConsoleOutput);


  Exit(0);
}
