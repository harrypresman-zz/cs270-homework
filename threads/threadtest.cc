// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

#ifdef HW1_TIME
#include <sys/time.h>
#include <time.h>
#endif

#if defined HW1_SEMAPHORES || defined HW1_LOCKS || defined HW1_CV
#include "synch.h"
#endif

// testnum is set in main.cc
int testnum = 1;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

#if defined THREADS && defined CHANGED
int SharedVariable; 

#if defined HW1_SEMAPHORES || defined HW1_LOCKS || defined HW1_CV
Barrier* barrier;
Lock* l;
Condition* c;
Semaphore* s;
#endif

#ifdef HW1_CV
void SignalThread(int which){ 
	int num, val=0;
	for(num = 0; num < 3; num++){ 
		currentThread->Yield(); 

		val = SharedVariable; 
		printf("*** SIGNAL thread %d sees value %d\n", which, val); 
		SharedVariable = val+1; 

		currentThread->Yield();
	}   

	l->Acquire();
	c->Signal(l);
	l->Release();  
}

void CondThread(int which){ 
	int num, val;

	l->Acquire();
	c->Wait(l);
	l->Release(); 
	for(num = 0; num < 5; num++){ 
		currentThread->Yield(); 
		l->Acquire();

		val = SharedVariable; 
		printf("*** COND thread %d sees value %d\n", which, val); 
		SharedVariable = val+1; 
		l->Release(); 
		currentThread->Yield();
	}
	l->Acquire();
	c->Signal(l);
	l->Release(); 

	barrier->block();
	val = SharedVariable; 
	printf("Thread %d sees final value %d\n", which, val); 
	barrier->block();
}
#endif  // end of CV

void SimpleThread(int which){ 
	int num, val; 
	for(num = 0; num < 5; num++){
#ifdef HW1_SEMAPHORES
		s->P();
#elif defined HW1_LOCKS
		l->Acquire();
#endif
		val = SharedVariable;
		printf("*** thread %d sees value %d\n", which, val); 
		currentThread->Yield(); 
		SharedVariable = val+1; 
#ifdef HW1_SEMAPHORES
		s->V();
#elif defined HW1_LOCKS
		l->Release();
#endif
		currentThread->Yield();
	}
#if defined HW1_SEMAPHORES || defined HW1_LOCKS
	barrier->block();
#endif
	val = SharedVariable; 
	printf("Thread %d sees final value %d\n", which, val); 
#if defined HW1_SEMAPHORES || defined HW1_LOCKS
	barrier->block();
#endif
}

#else  // not changed or threads
void SimpleThread(int which){
	int num;

	for (num = 0; num < 5; num++) {
		printf("*** thread %d looped %d times\n", which, num);
		currentThread->Yield();
	}
}
#endif

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------
#if defined THREADS && defined CHANGED 
#ifdef HW1_CV 
void ThreadTest1(int n){
	DEBUG('t', "Entering ThreadTest1");
	barrier = new Barrier("b", n+1);
	l = new Lock("lock");
	c = new Condition("Cond");
	for(int i = 1; i <= n; i++){
		Thread *t = new Thread("forked thread");
		t->Fork(CondThread, i);
	}
	currentThread->Yield();
	l->Acquire(); 
	SignalThread(0);
}

#else
void ThreadTest1(int n){
	DEBUG('t', "Entering ThreadTest1");
#if defined HW1_SEMAPHORES || defined HW1_LOCKS
	s = new Semaphore("s", 1);
	l = new Lock("l");
	barrier = new Barrier("b", n+1);
#endif
	for(int i = 1; i <= n; i++){
		Thread *t = new Thread("forked thread");
		t->Fork(SimpleThread, i);
	}

	SimpleThread(0);
}
#endif  // not cv, semaphores or locks
#else   // not changed or not threads
void ThreadTest1(){
	DEBUG('t', "Entering ThreadTest1");

	Thread *t = new Thread("forked thread");

	t->Fork(SimpleThread, 1);
	SimpleThread(0);
}
#endif

#ifdef HW1_TIME
long elapsedTime(struct timeval start, struct timeval end){

	// calculate time in microseconds
	long tS = start.tv_sec*1000000 + start.tv_usec;
	long tE = end.tv_sec*1000000  + end.tv_usec;
	return tE - tS;
}

int switchCount;
#define EIGHT 8388608

void SwitchingOnNumber(int size){
	char* temp = new char[EIGHT];
	int num = 0, counter = 0;

	for(; counter < 100; num++){
		time_t t = time(NULL);
		char c = temp[num%EIGHT];
//		temp[num%EIGHT] = ++c;
		if(num%size == 0){
			switchCount++;
			currentThread->Yield();
			counter++;
		}
	}
	switchCount++;
	currentThread->Yield();
}

void SwitchingOffNumber(int size){
	char *temp = new char[EIGHT];
	int num = 0, counter = 0;

	for(; counter < 100; num++){
		time_t t = time(NULL);
		char c = temp[num%EIGHT];
//		temp[num%EIGHT] = ++c;
		if(num%size == 0){
			counter++;
		}
		num++;
	}
	switchCount++;
	currentThread->Yield();

}

void SwitchingOnSize(int size){
	char *temp = new char[EIGHT];

	for(int num = 0; num < 10000000; num++) {
		char c = temp[num%EIGHT];
		if( num%size == 0){
			switchCount++;
			currentThread->Yield();          
		}
	}
	currentThread->Yield();
	switchCount++;
}

void SwitchingOffSize(int size){
	char *temp = new char[EIGHT];

	for(int num = 0; num < 10000000; num++) {
		char c = temp[num%EIGHT];
		if( num%size == 0){
			// nop
		}
	}
	currentThread->Yield();
	switchCount++;
}

void TimingThreadTest1(int size){

	Thread *t = new Thread("forked thread");

	struct timeval start,end;
	gettimeofday(&start,NULL);

	t->Fork(SwitchingOnNumber, size);
	SwitchingOnNumber(size);

	currentThread->Yield();

	gettimeofday(&end,NULL);
	long switchTime = elapsedTime(start,end);  

	printf("\n");
	int oldSwitches = switchCount;
	switchCount = 0;
	t = new Thread("forked thread");

	gettimeofday(&start,NULL);

	t->Fork(SwitchingOffNumber, size);
	SwitchingOffNumber(size);

	currentThread->Yield();

	gettimeofday(&end,NULL);
	long noSwitchTime = elapsedTime(start,end);  

	double aveTime = (switchTime - noSwitchTime)/(oldSwitches/2.0);

	printf("%d size\n", size);
	printf("%d switches, %ld switch time, %ld no switch time, %ld us\n", 
		oldSwitches/2, switchTime, noSwitchTime, switchTime - noSwitchTime);  
	printf("%d switches took an average of %.2lf us\n", oldSwitches/2, aveTime);

}

void TimingThreadTest2(int size){

	Thread *t = new Thread("forked thread");

	struct timeval start,end;
	gettimeofday(&start,NULL);

	t->Fork(SwitchingOnSize, size);
	SwitchingOnSize(size);

	currentThread->Yield();

	gettimeofday(&end,NULL);
	long switchTime = elapsedTime(start,end);  

	printf("\n");
	int oldSwitches = switchCount;
	switchCount = 0;
	t = new Thread("forked thread");

	gettimeofday(&start,NULL);

	t->Fork(SwitchingOffSize, size);
	SwitchingOffSize(size);

	currentThread->Yield();

	gettimeofday(&end,NULL);
	long noSwitchTime = elapsedTime(start,end);  

	double aveTime = (switchTime - noSwitchTime)/(oldSwitches/2.0);

	printf("%d size\n", size);
	printf("%d switches, %ld switch time, %ld no switch time, %ld us\n", 
		oldSwitches/2, switchTime, noSwitchTime, switchTime - noSwitchTime);  
	printf("%d switches took an average of %.2lf us\n", oldSwitches/2, aveTime);

}
#endif

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------
#if defined CHANGED && defined THREADS
void ThreadTest(int n){
	switch (testnum) {
		case 1:
#ifdef HW1_TIME        
/*
			TimingThreadTest1(1024);
			TimingThreadTest1(8196);
			TimingThreadTest1(8196*2);
			TimingThreadTest1(8196*4);
			TimingThreadTest1(8196*16);
			TimingThreadTest1(262144);
			TimingThreadTest1(262144*8);
			TimingThreadTest1(8388608);
*/
			TimingThreadTest2(1024);
			TimingThreadTest2(8196);
			TimingThreadTest2(8196*2);
			TimingThreadTest2(8196*4);
			TimingThreadTest2(8196*16);
			TimingThreadTest2(262144);
			TimingThreadTest2(262144*8);
			TimingThreadTest2(8388608);

#else
			ThreadTest1(n);
#endif
			break;
		default:
			printf("No test specified.\n");
			break;
	}
}
#else
void ThreadTest(){
	switch (testnum) {
		case 1:
			ThreadTest1();
			break;
		default:
			printf("No test specified.\n");
			break;
	}
}
#endif


