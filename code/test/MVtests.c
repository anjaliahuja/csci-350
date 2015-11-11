#include "syscall.h"

int lock2, mv1, cv1, value, newVal;

int main(){

	lock2 = CreateLock("lock2", sizeof("lock2"));
	mv1 = CreateMV("mv1", 2, 1);
	cv1 = CreateCV("cv1", 2);

	Acquire(lock2);
	Write("Get value of MV\n", sizeof("Get value of MV\n"), ConsoleOutput);

	value = GetMV(mv1, 0);
	Printf("Value of monitor variable at position 0: %d\n",sizeof("Value of monitor variable at position 0: %d\n"), value);

	Write("Setting MV to increment\n", sizeof("Setting MV to increment\n"), ConsoleOutput);
	SetMV(mv1, 0, value+1);
	newVal = GetMV(mv1, 0);

	Printf("Value of MV after incrementing: %d\n", sizeof("Value of MV after incrementing: %d\n"), newVal);

	Wait(lock2, cv1);

	value = GetMV(mv1, 0);
	Printf("Value of monitor variable after broadcast at position 0: %d\n",sizeof("Value of monitor variable after broadcast at position 0: %d\n"), value);



	
	/*DestroyLock(lock2);
	//Write("Lock destroyed\n", sizeof("Lock destroyed\n"), ConsoleOutput);
	

	//DestroyMV(mv1);
		//Write("MV destroyed\n", sizeof("MV destroyed\n"), ConsoleOutput);
	*/
	Release(lock2);

	Exit(0);


}