/* halt.c
 *	Simple program to test whether running a user program works.
 *	
 *	Just do a "syscall" that shuts down the OS.
 *
 * 	NOTE: for some reason, user programs with global data structures 
 *	sometimes haven't worked in the Nachos environment.  So be careful
 *	out there!  One option is to allocate data structures as 
 * 	automatics within a procedure, but if you do this, you have to
 *	be careful to allocate a big enough stack to hold the automatics!
 */

#include "syscall.h"

void testFunc(){
    int x = 5;
    x += 5;
    x -= 2;
    Exit(0);
}

int main(){
    int pid;
    
    Fork(testFunc);
    Exec("test/halt2");
    pid = Exec("test/halt2");
    Join(pid);
    Create("apple");
    //Open("apple");
    Read(0, 1, 1);
    Write(0, 1, 1);
    Close(1);

    //Fork(testFunc);
    Yield();
    Halt();
    /* not reached */
}
