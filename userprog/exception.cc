// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

#define PCReg		34	// Current program counter
#define NextPCReg	35	// Next program counter (for branch delay) 
#define PrevPCReg	36	// Previous program counter (for debugging)

void myFork(int newPC);
void myDummyFork(int newPC);

void incrementPC(){
    machine->registers[PrevPCReg] = machine->registers[PCReg];
    machine->registers[PCReg] = machine->registers[NextPCReg];
    machine->registers[NextPCReg] = machine->registers[PCReg] + 4;
}

void myFork(int newPC){
    DEBUG('a', "FORK, initiated by user program.\n");
    // fork kernel thread
    Thread * forkedThread = new Thread("ForkedThread");
    //forkedThread->space = currentThread->space;
    forkedThread->Fork(myDummyFork, newPC);
    currentThread->Yield();
}

void myDummyFork(int newPC){
    // set addr space to dup of this thread space
    // sets
    // yield
    // reg copying stuff, etc
    DEBUG('a', "FORK, forked thread.\n");
    incrementPC();
}

void ExceptionHandler(ExceptionType which){
    int type = machine->ReadRegister(2);

    int arg1, arg2, arg3, arg4;

    if ((which == SyscallException) && (type == SC_Halt)) {
        DEBUG('a', "Shutdown, initiated by user program.\n");
        interrupt->Halt();
    }else if ((which == SyscallException) && (type == SC_Exit)) {
        DEBUG('a', "EXIT, initiated by user program.\n");
    }else if ((which == SyscallException) && (type == SC_Exec)) {
        DEBUG('a', "EXEC, initiated by user program.\n");
    }else if ((which == SyscallException) && (type == SC_Join)) {
        DEBUG('a', "JOIN, initiated by user program.\n");
    }else if ((which == SyscallException) && (type == SC_Create)) {
        DEBUG('a', "CREATE, initiated by user program.\n");
    }else if ((which == SyscallException) && (type == SC_Open)) {
        DEBUG('a', "OPEN, initiated by user program.\n");
    }else if ((which == SyscallException) && (type == SC_Read)) {
        DEBUG('a', "READ, initiated by user program.\n");
    }else if ((which == SyscallException) && (type == SC_Write)) {
        DEBUG('a', "WRITE, initiated by user program.\n");
    }else if ((which == SyscallException) && (type == SC_Close)) {
        DEBUG('a', "CLOSE, initiated by user program.\n");
    }else if ((which == SyscallException) && (type == SC_Fork)) {
        arg1 = machine->ReadRegister(4);
        myFork( arg1 );
    }else if ((which == SyscallException) && (type == SC_Yield)) {
        DEBUG('a', "YIELD, initiated by user program.\n");
    } else {
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(FALSE);
    }
    incrementPC();
}
