#include "syscall.h"

int main () {
	Exec("../test/passportsim", sizeof("../test/passportsim"));
  Write("hello\n", 6, ConsoleOutput);
}
