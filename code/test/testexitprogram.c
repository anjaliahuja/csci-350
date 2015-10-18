#include "syscall.h"

void g1() {
	Printf0("g1\n", sizeof("g1\n"));
	Exit(0);
}

void g2() {
	Printf0("g2\n", sizeof("g2\n"));
	Exit(0);
}

void g3() {
	Printf0("g3\n", sizeof("g3\n"));
	Exit(0);
}

void g4() {
	Printf0("g4\n", sizeof("g4\n"));
	Exit(0);
}

void g5() {
	Printf0("g5\n", sizeof("g5\n"));
	Exit(0);
}

int main() {
	Printf0("m2\n", sizeof("m2\n"));

	Fork(g1, "nameme", sizeof("nameme"));
	Fork(g2, "nameme", sizeof("nameme"));
	Fork(g3, "nameme", sizeof("nameme"));
	Fork(g4, "nameme", sizeof("nameme"));
	Fork(g5, "nameme", sizeof("nameme"));

}