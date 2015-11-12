/* testfiles.c
 *  Simple program to test the file handling system calls
 */

#include "syscall.h"

int i, lock, cv;

int main() {
  Write("Test 6 creates lock and cv\n", sizeof("Test 6 creates lock and cv\n"), ConsoleOutput);
  lock = CreateLock("lock", sizeof("lock"));
  cv = CreateCV("cv", sizeof("cv"));

  for (i = 0; i < 50000; i++) {
    Yield();
  }

  Write("Test 6 tries to acquire lock and wait on cv (both destroyed)\n", sizeof("Test 6 tries to acquire lock and wait on cv (both destroyed)\n"), ConsoleOutput);

  Acquire(lock);
  Wait(lock, cv);
}
