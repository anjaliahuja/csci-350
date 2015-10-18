#include "syscall.h"

void t1(){
	Write("t1\n", sizeof("t1\n"), ConsoleOutput);
	Exit(0);
}

void t2(){
	Write("t2\n", sizeof("t1\n"), ConsoleOutput);
	Exit(0);
}

void t3(){
	Write("t3\n", sizeof("t1\n"), ConsoleOutput);
	Exit(0);
}





int main(){
	Exec("../test/testexitprogram", sizeof("../test/testexitprogram"));
	/*Exec("../test/testexitprogram", sizeof("../test/testexitprogram"));
	*/

	Fork(t1, "name", sizeof("name"));	
	Fork(t2, "name", sizeof("name"));
	Fork(t3, "name", sizeof("name"));





}