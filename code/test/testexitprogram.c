#include "syscall.h"

void g1() {
	Write("Fork g1 from exec\n", sizeof("Fork g1 from exec\n"), ConsoleOutput);
	Exit(0);
}

void g2() {
	Write("Fork g2 from exec\n", sizeof("Fork g2 from exec\n"), ConsoleOutput);
	Exit(0);
}

void g3() {
	Write("Fork g3 from exec\n", sizeof("Fork g3 from exec\n"), ConsoleOutput);
	Exit(0);
}


int main() {
	Write("Main from new executed process\n", sizeof("Main from new executed process\n"), ConsoleOutput);

	Fork(g1, "nameme", sizeof("nameme"));
	Fork(g2, "nameme", sizeof("nameme"));
	Fork(g3, "nameme", sizeof("nameme"));
}