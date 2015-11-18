#include "syscall.h"

int lock2, mv1, cv1, value, newVal;

int main(){

	lock2 = CreateLock("lock2", sizeof("lock2"));
	mv1 = CreateMV("mv1", 2, 1);
	cv1 = CreateCV("cv1", 2);

	Acquire(lock2);
	Write("This test will broadcast all clients waiting on cv1.\n", sizeof("This test will broadcast all clients waiting on cv1.\n"), ConsoleOutput);

	Broadcast(lock2, cv1);
	

	Release(lock2);

	Exit(0);



}