/* testfiles.c
 *  Simple program to test the file handling system calls
 */

#include "syscall.h"

int lock;
int cv;

int main() {
  lock = CreateLock("lock", 4); 
  cv = CreateCV("cv", 2);
  Write("Test 2 Acquires Lock\n", sizeof("Test 2 Acquires Lock\n"), ConsoleOutput);
  Acquire(lock);
  Write("Test 2 Waits on Lock\n", sizeof("Test 2 Waits on Lock\n"), ConsoleOutput);
  Wait(lock, cv);
  Release(lock);
}
