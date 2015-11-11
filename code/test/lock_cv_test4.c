/* testfiles.c
 *  Simple program to test the file handling system calls
 */

#include "syscall.h"

int lock;
int cv;

int main() {
    lock = CreateLock("lock", 4);
cv = CreateCV("cv", 2);
    Write("Test 4 broadcasts all locks\n", sizeof("Test 4 broadcasts locks\n"), ConsoleOutput);

  Broadcast(lock, cv);
  Release(lock);
}
