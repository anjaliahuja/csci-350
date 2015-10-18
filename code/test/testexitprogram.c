#include "syscall.h"

void g1() {
	Write("g1\n", sizeof("g1\n"), ConsoleOutput);
	Exit(0);
}

void g2() {
	Write("g2\n", sizeof("g2\n"), ConsoleOutput);
	Exit(0);
}

void g3() {
	Write("g3\n", sizeof("g3\n"), ConsoleOutput);
	Exit(0);
}


int main() {
	Write("m2\n", sizeof("m2\n"), ConsoleOutput);

	Fork(g1, "nameme", sizeof("nameme"));
	Fork(g2, "nameme", sizeof("nameme"));
	Fork(g3, "nameme", sizeof("nameme"));
}