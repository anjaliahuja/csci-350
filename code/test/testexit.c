#include "syscall.h"

void t1(){
	Printf("t1\n", sizeof("t1\n"));
	Exit(0);
}

void t2(){
	Printf("t2\n", sizeof("t1\n"));
	Exit(0);
}

void t3(){
	Printf("t3\n", sizeof("t1\n"));
	Exit(0);
}





int main(){
	Exec("../test/testexitprogram", sizeof(../test/testexitprogram));
	Exec("../test/testexitprogram", sizeof(../test/testexitprogram));
	Exec("../test/testexitprogram", sizeof(../test/testexitprogram));


	Fork(t1, "name", sizeof("name"));	
	Fork(t2, "name", sizeof("name"));
	Fork(t3, "name", sizeof("name"));





}