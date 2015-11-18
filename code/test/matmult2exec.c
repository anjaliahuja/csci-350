#include "syscall.h"

int main() {
  Exec("../test/matmult", sizeof("../test/matmult"));
  Exec("../test/matmult", sizeof("../test/matmult"));
}
