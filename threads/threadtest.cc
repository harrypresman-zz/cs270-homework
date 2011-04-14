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

#if defined HW1_SEMAPHORES || defined HW1_LOCKS
#include "synch.h"
#endif

void Test(){
    int i;
    printf("Hello");
}

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

#ifdef HW1_SEMAPHORES
Semaphore *s;
int loadCount, numThreads;
bool loadDone;
#elif defined HW1_LOCKS
Lock *l;
int loadCount, numThreads;
bool loadDone;
#endif

#if defined CHANGED && defined THREADS
int SharedVariable;

void
SimpleThread(int which){
    int num, val;
    
    for (num = 0; num < 5; num++) {
#ifdef HW1_SEMAPHORES
        s->P();
#elif defined HW1_LOCKS
        l->Acquire();
#endif
        val = SharedVariable;
	    printf("*** thread %d sees value %d\n", which, val);
        currentThread->Yield();

        SharedVariable = val + 1;

#ifdef HW1_SEMAPHORES
        s->V();
#elif defined HW1_LOCKS
        l->Release();
#endif
        currentThread->Yield();
    }
#if defined HW1_SEMAPHORES || defined HW1_LOCKS
    loadCount--;
    if(loadCount == 0){
        loadCount = numThreads;
        loadDone = !loadDone;
    }
    //Load barrier
    while(!loadDone) currentThread->Yield();
#endif
    val = SharedVariable;
    printf("Thread %d sees final value %d\n", which, val);
#if defined HW1_SEMAPHORES || defined HW1_LOCKS
    loadCount--;
    if(loadCount == 0){
        loadCount = numThreads;
        loadDone = !loadDone;
    }
    //Load barrier
    while(loadDone) currentThread->Yield();
#endif
}

#else
void
SimpleThread(int which)
{
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

#if defined CHANGED && defined THREADS
void
ThreadTest1(int n){
    DEBUG('t', "Entering ThreadTest1");
#ifdef HW1_SEMAPHORES
    s = new Semaphore("s", 1);
    loadCount = numThreads = n + 1;
    loadDone = false;
#elif defined HW1_LOCKS
    l = new Lock("l");
    loadCount = numThreads = n + 1;
    loadDone = false;
#endif
    int i = 1;
    for( i; i <= n; i++){
        Thread *t = new Thread("forked thread");
        t->Fork(SimpleThread, i);
    }
    SimpleThread(0);
}
#else
void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}
#endif


long elapsedTime(struct timeval start, struct timeval end){

    // calculate time in microseconds
    long tS = start.tv_sec*1000000 + (start.tv_usec);
    long tE = end.tv_sec*1000000  + (end.tv_usec);
    return tE - tS ;
}

int switchCount;

void TimingThreadSwitch(int size){
    char *temp = new char[size];
    int x;
    
    for (int num = 0, x = 0; num < 1000000; num++, x=(x + 4096)%size) {
//        for(int x = 0; x < size; x++ ){
            time_t t = time(NULL);
            char c = temp[x];
            temp[x] = c++;
            switchCount++;
            currentThread->Yield();          
//        }       
    }
    currentThread->Yield();
    switchCount++;
}

void TimingThreadNoSwitch(int size){
    char *temp = new char[size];
    int x;
    
    for (int num = 0, x = 0; num < 1000000; num++, x=(x + 4096)%size) {
//        for(int x = 0; x < size; x++ ){
            time_t t = time(NULL);
            char c = temp[x];
            temp[x] = c++;
//        }        
    }
    currentThread->Yield();
    switchCount++;
}

void TimingThreadTest(int size){
    
    Thread *t = new Thread("forked thread");

    struct timeval start,end;
    gettimeofday(&start,NULL);
    
    t->Fork(TimingThreadSwitch, size);
    TimingThreadSwitch(size);
    
    currentThread->Yield();
    
    gettimeofday(&end,NULL);
    long switchTime = elapsedTime(start,end);  
    
    printf("\n");
    int oldSwitches = switchCount;
    switchCount = 0;
    t = new Thread("forked thread");

    gettimeofday(&start,NULL);
    
    t->Fork(TimingThreadNoSwitch, size);
    TimingThreadNoSwitch(size);
    
    currentThread->Yield();
    
    gettimeofday(&end,NULL);
    long noSwitchTime = elapsedTime(start,end);  
    
    double aveTime = (switchTime - noSwitchTime)/(oldSwitches/2.0);
    
    printf("%d switches, %ld switch time, %ld no switch time, %ld us\n", oldSwitches, switchTime, noSwitchTime, switchTime - noSwitchTime);  
    printf("%d switches took an average of %.2lf us\n", oldSwitches/2, aveTime);
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------
#if defined CHANGED && defined THREADS
void
ThreadTest(int n)
{
    switch (testnum) {
    case 1:
        #ifdef HW1_TIME        
        TimingThreadTest(1024);
//        TimingThreadTest(1024, 1000);
        TimingThreadTest(262144);
//        TimingThreadTest(262144, 1000);
        TimingThreadTest(8388608);
//        TimingThreadTest(8388608, 1000);
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
void
ThreadTest()
{
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
