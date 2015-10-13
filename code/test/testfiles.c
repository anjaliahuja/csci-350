/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

int t1_l1;

void t1() {

  Acquire(t1_l1);
  Write("t1_t1 acquired t1_l1\n", sizeof("t1_t1 acquired t1_l1\n"), ConsoleOutput);

  Write("t1_t1 releasing t1_l1\n", sizeof("t1_t1 releasing t1_l1\n"), ConsoleOutput);
  Release(t1_l1);

  Exit(0);
}

int main() {
  OpenFileId fd;
  int bytesread;
  char buf[20];

  Create("testfile", 8);
  fd = Open("testfile", 8);

  Write("testing a write\n", 16, fd );
  Close(fd);


  fd = Open("testfile", 8);
  bytesread = Read( buf, 100, fd );
  Write( buf, bytesread, ConsoleOutput );
  Close(fd);

  t1_l1 = CreateLock("t1_l1", 5); 

  Write("Test 1\n", sizeof("Test 1\n"), ConsoleOutput);
  Fork(t1, "t1_t1", sizeof("t1_t1"));
  Write("End Test 1\n", sizeof("End Test 1\n"), ConsoleOutput);

}
