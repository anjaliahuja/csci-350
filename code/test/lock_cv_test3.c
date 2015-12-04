/* testfiles.c
 *  Simple program to test the file handling system calls
 */

#include "syscall.h"

int lock;
int cv;

int main() {
  lock = CreateLock("lock", 4);
  cv = CreateCV("cv", 2);
  Write("Test 3 Acquires Lock\n", sizeof("Test 3 Acquires Lock\n"), ConsoleOutput);
  Acquire(lock);
  Write("Test 3 signals lock\n", sizeof("Test 3 signals lock\n"), ConsoleOutput);
  Signal(lock, cv);
  Write("Test 3 Releases lock\n", sizeof("Test 3 Releases lock\n"), ConsoleOutput);

  Release(lock);
  Exit(0);
}
