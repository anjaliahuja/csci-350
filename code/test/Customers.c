
#include "Setup.h"
#include "syscall.h"

int main() {
  int i = 0;
  for (i; i < NUM_CUSTOMERS; i++) 
    Exec("../test/Customer", sizeof("../test/Customer"));

  Exit(0);
}
