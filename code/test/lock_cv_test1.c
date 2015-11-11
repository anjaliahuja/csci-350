/* testfiles.c
 *  Simple program to test the file handling system calls
 */

#include "syscall.h"

int lock;

int main() {
  lock = CreateLock("lock", 4); 

  Acquire(lock);
  Write("Test 1 acquires lock\n", sizeof("Test 1 acquires lock\n"), ConsoleOutput);

  Write("Test 1 releases lock\n", sizeof("Test 1 releases lock\n"), ConsoleOutput);
  Release(lock);


  Exit(0);
}
