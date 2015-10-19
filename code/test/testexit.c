#include "syscall.h"

void t1(){
	Write("Fork t1\n", sizeof("Fork t1\n"), ConsoleOutput);
	Exit(0);
}

void t2(){
	Write("Fork t2\n", sizeof("Fork t2\n"), ConsoleOutput);
	Exit(0);
}

void t3(){
	Write("Fork t3\n", sizeof("Fork t3"), ConsoleOutput);
	Exit(0);
}





int main(){
	/*Executes two new processes*/
	Exec("../test/testexitprogram", sizeof("../test/testexitprogram"));
	Exec("../test/testexitprogram", sizeof("../test/testexitprogram"));
	

	/*Forks 3 threads*/
	Fork(t1, "name", sizeof("name"));	
	Fork(t2, "name", sizeof("name"));
	Fork(t3, "name", sizeof("name"));





}