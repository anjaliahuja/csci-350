#include "syscall.h"

int main(){
	int lock, mv, cv, value, newVal;

	lock = CreateLock("lock", sizeof("lock"));
	mv = CreateMV("mv", 2, 1);
	cv = CreateCV("cv", 2);

	Acquire(lock);
	Write("Get value of MV\n", sizeof("Get value of MV\n"), ConsoleOutput);

	value = GetMV(mv, 0);
	Printf("Value of monitor variable at position 0: %d\n",sizeof("Value of monitor variable at position 0: %d\n"), value);

	Write("Setting MV to increment\n", sizeof("Setting MV to increment\n"), ConsoleOutput);
	SetMV(mv, 0, value+1);
	newVal = GetMV(mv, 0);

	Printf("Value of MV after incrementing: %d\n", sizeof("Value of MV after incrementing: %d\n"), newVal);

	Write("Lock released", sizeof("Lock released"), ConsoleOutput);
	
	Wait(lock, cv);

	DestroyLock(lock);
	Write("Lock destroyed\n", sizeof("Lock destroyed\n"), ConsoleOutput);
	
	Release(lock);

	DestroyMV(mv);
		Write("MV destroyed\n", sizeof("MV destroyed\n"), ConsoleOutput);


	Exit(0);

}