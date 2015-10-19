#include syscall.h

void test_print(){
	int random;
	random = Rand(10, 5);
	Printf("Printing out random number between 5 and 14: %d\n", sizeof("Printing out random number between 5 and 14: %d\n"), random);
	Write("Testing print syscall \n", sizeof("Testing print syscall \n"), ConsoleOutput);
	Printf("Printf is printing a string with one number: %d\n", sizeof("Printf is printing a string with one number"), 1);
	Write("Testing printf with 2 numbers, 1 and 3\n", sizeof("Testing printf with 2 numbers, 1 and 3"), ConsoleOutput);
	Printf("Printf is printing num 1: %d, and num 2: %d\n", sizeof("Printf is printing num 1: %d, and num 2: %d\n"), 3*1000+1);
	Exit(0);
}

int main(){
	Fork(test_print, "testprint", sizeof("testprint"));
	Yield();
}