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

#define RetAddrReg	31	// Holds return address for procedure calls
#define PCReg		34	// Current program counter
#define NextPCReg	35	// Next program counter (for branch delay) 
#define PrevPCReg	36	// Previous program counter (for debugging)

#define READ_OP 1
#define WRITE_OP 2

void myFork( int newPC );
void forkBridge( int newPC );
void myExec( int vAddr );
void execBridge( int newPC );

void incrementPC(){
    machine->registers[PrevPCReg] = machine->registers[PCReg];
    machine->registers[PCReg] = machine->registers[NextPCReg];
    machine->registers[NextPCReg] = machine->registers[PCReg] + 4;
}

void jumpPC( int newPC ){
    machine->registers[PCReg] = newPC;
    machine->registers[NextPCReg] = newPC + 4;
    // TODO: do we need to do anything with the ret addr??
    //machine->registers[RetAddrReg] += 8;
}

char* getString( int vAddr ){
    int pAddr;
    // get the mem addr from register that holds exec name as string
    currentThread->space->Translate( vAddr, &pAddr ); 

    char* str = new char[ strlen( machine->mainMemory + pAddr )+1 ]; // create
    strcpy( str, machine->mainMemory + pAddr ); // we need a char* from mem

    return str;
}

void putString( char* str, int vAddr ){
    int pAddr;
    currentThread->space->Translate( vAddr, &pAddr );

    strcpy( machine->mainMemory + pAddr, str );
}

void getMemFromVAddr( char* buffer, int vAddr, int size ){
    int pAddr;
    currentThread->space->Translate( vAddr, &pAddr ); 
    memcpy( buffer, machine->mainMemory + pAddr, size );
}

void putMemIntoVAddr( char* buffer, int vAddr, int size ){
    int pAddr;
    currentThread->space->Translate( vAddr, &pAddr ); 
    memcpy( machine->mainMemory + pAddr, buffer, size ); 
}

void myExit(int exitStatus){
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    DEBUG('t', "EXIT, initiated by user program %s pid:%d parentPID:%d.\n", 
            currentThread->getName(), currentThread->space->pcb->PID, 
            currentThread->space->pcb->parentPID );
    int pid = currentThread->space->pcb->PID;
    procMgr->setExitStatus( currentThread->space->pcb->PID, exitStatus );
    
    DEBUG('2', "Process %d exits with %d\n", pid , exitStatus); //TODO this is the parent PID #4
    delete currentThread->space;
    currentThread->Finish();

    machine->WriteRegister( 2, exitStatus );
    interrupt->SetLevel(oldLevel);
}

void myExec( int vAddr ){
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    char* file = getString( vAddr );
    OpenFile* executable = fileSystem->Open( file );

    if( executable == NULL ){
        DEBUG('t', "Unable to open file %s\n", file );
        return;
    }
    Thread* forkedThread;
    forkedThread = new Thread( "Exec Thread" );
    AddrSpace* space = new AddrSpace( executable );
    forkedThread->space = space;   
    // in test3.c -- exec on test3_1
    // something is going wrong, space is getting the address 0x100 
    // after the Yield for some reason!!

    forkedThread->space->pcb->parentPID = currentThread->space->pcb->PID;
    forkedThread->space->pcb->thread = forkedThread;
    delete file;
    delete executable;			// close file

    forkedThread->Fork( execBridge, 0 );
    DEBUG('t', "EXEC, initiated by user program. %s myPID: %d parentPID:%d \n", 
            currentThread->getName(), currentThread->space->pcb->PID, 
            currentThread->space->pcb->parentPID );
    DEBUG('2', "Exec Program: %d loading %s\n", forkedThread->space->pcb->PID , file); //TODO is this the right PID?
    
    machine->WriteRegister( 2, forkedThread->space->pcb->PID );
    currentThread->Yield();
    interrupt->SetLevel(oldLevel);
}

void execBridge( int newPC ){
    currentThread->space->InitRegisters();		// set the initial register values
    currentThread->space->RestoreState();		// load page table register

    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
    // the address space exits
    // by doing the syscall "exit"
    // printf( "EXEC, initiated by user program %s.\n", 
    // currentThread->getName() );
}

void myFork( int newPC ){
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    // fork kernel thread
    Thread * forkedThread = new Thread("ForkedThread");
    forkedThread->space = new AddrSpace;
    currentThread->space->CopyAddrSpace( forkedThread->space );
    int newPID = procMgr->getPID();
    forkedThread->space->pcb = new PCB(newPID, currentThread->space->pcb->PID,
            forkedThread);
    procMgr->storePCB( forkedThread->space->pcb );
    DEBUG('t', "FORK, initiated by user program %s. pid:%d parentPID:%d \n", 
            currentThread->getName(), currentThread->space->pcb->PID, 
            currentThread->space->pcb->parentPID );
    forkedThread->Fork( forkBridge, newPC );
    currentThread->Yield();
    DEBUG('t',"returned from fork\n");
    machine->WriteRegister(2, newPID);
    interrupt->SetLevel(oldLevel);
}

void forkBridge( int newPC ){
    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();

    DEBUG('t', "FORK, forked thread %s.\n", currentThread->getName() );
    jumpPC(newPC);

    machine->Run();
}

/*
 * FILESYSTEM CALLS
 */

void myCreate( int vAddr ){
    DEBUG('t',"CREATE, initiated by user program %s.\n", 
            currentThread->getName() );
    char* fileName = getString( vAddr );
    fileSystem->Create( fileName, 0 );
}

void myOpen( int vAddr ){
    char* fileName = getString( vAddr );
    
    OpenFile* file = fileSystem->Open( fileName );
    if( file == NULL ){
        myCreate( vAddr ); // file not found!!
        file = fileSystem->Open( fileName );
    }

    SysOpenFile* sysOpenFile = procMgr->getOpenFile( fileName, file );

    if( sysOpenFile == NULL ) return; // no open spots left

    UserOpenFile* userOpenFile = currentThread->space->pcb->getOpenFile( fileName, sysOpenFile );

    machine->WriteRegister( 2, sysOpenFile->fd );
    DEBUG('t', "OPEN %s, initiated by user program %s, assigned FD %d.\n", 
            fileName, currentThread->getName(), userOpenFile->fd );
}

void myClose( int fd ){
    DEBUG('t', "CLOSE FD %d, initiated by user program %s.\n",
            fd, currentThread->getName() );
    currentThread->space->pcb->closeFile( fd );
    procMgr->closeFile( fd );
}

int myRead( int vAddr, int size, OpenFileId fd ){
//    DEBUG('t', "READ, initiated by user program %s.\n", 
//            currentThread->getName() );


    if( fd == ConsoleInput ){
        for( int i = 0; i < size; i++ ){
            diskBuffer[i] = getchar(); 
        }
	//Sample trimmed excess end line chars. is this needed?  
	//char next = 'a';
	// Get rid of excess characters
	//while ((next != '\n')&&(next != EOF)) {
	//	  next = getchar();
	// }
        putMemIntoVAddr( diskBuffer, vAddr, size );
        machine->WriteRegister( 2, 1 );
    }else{
        UserOpenFile* file = currentThread->space->pcb->getOpenFile( fd );
        int pos = file->position;
        int numRead = 0;
        int totNumRead = 0;
        while( size > 0 ){
            int pSize = (size > PageSize) ? PageSize : size;
            numRead = file->sysOpenFile->openFile->ReadAt( diskBuffer, pSize, pos );
            totNumRead += numRead;
            putMemIntoVAddr( diskBuffer, vAddr, numRead );
            pos += pSize;
            size -= pSize;
            vAddr += pSize;
        }
        //TODO: do we need this?
        char zero[1];
        zero[0] = '\0';
        putMemIntoVAddr( zero, vAddr, 1 );
        numRead = totNumRead;
        file->position = pos;

        machine->WriteRegister( 2, numRead );
    }
    return 0;
}

void myWrite( int vAddr, int size, OpenFileId fd ){
//    printf( "WRITE, initiated by user program %s.\n", 
//            currentThread->getName() );
    if( fd == ConsoleOutput ){
        char* buffer = getString( vAddr );
        printf("%s", buffer );
    }else{
        UserOpenFile* file = currentThread->space->pcb->getOpenFile( fd );
        int pos = file->position;
        int numRead = 0;
        while( size > 0 ){
            int pSize = (size > PageSize) ? PageSize : size;
            getMemFromVAddr( diskBuffer, vAddr, pSize );
            numRead = file->sysOpenFile->openFile->WriteAt( diskBuffer, pSize, pos );
            pos += pSize;
            size -= pSize;
            vAddr += pSize;
        }
        file->position = pos;
    }
}



void ExceptionHandler( ExceptionType which ){
    int type = machine->ReadRegister( 2 );

    int arg1 = machine->ReadRegister( 4 );
    int arg2 = machine->ReadRegister( 5 );
    int arg3 = machine->ReadRegister( 6 );
    int arg4 = machine->ReadRegister( 7 ); // never used
    int pid = currentThread->space->pcb->PID;
    if( (which == SyscallException) && (type == SC_Halt) ){
	DEBUG('2',"System Call: %d invoked Halt\n", pid);
        DEBUG('t', "Shutdown, initiated by user program %s.\n", 
                currentThread->getName() );
        interrupt->Halt();
    }else if( (which == SyscallException) && (type == SC_Exit) ){
        DEBUG('2',"System Call: %d invoked Exit\n", pid);        
        myExit( arg1 );
    }else if( (which == SyscallException) && (type == SC_Exec) ){
	DEBUG('2',"System Call: %d invoked Exec\n", pid);        
        myExec( arg1 );
    }else if( (which == SyscallException) && (type == SC_Join) ){
        DEBUG('2',"System Call: %d invoked Join\n", pid);        
	DEBUG('t', "JOIN, initiated by user program %s. Waiting on pid:%d \n", 
                currentThread->getName(), arg1 );
        procMgr->join( arg1 );
    }else if( (which == SyscallException) && (type == SC_Create) ){
	DEBUG('2',"System Call: %d invoked Create\n", pid);        
        myCreate( arg1 );
    }else if( (which == SyscallException) && (type == SC_Open) ){
	DEBUG('2',"System Call: %d invoked Open\n", pid);        
        myOpen( arg1 );
    }else if( (which == SyscallException) && (type == SC_Read) ){
        DEBUG('2',"System Call: %d invoked Read\n", pid);
        myRead( arg1, arg2, arg3 );
    }else if( (which == SyscallException) && (type == SC_Write) ){
        DEBUG('2',"System Call: %d invoked Write\n", pid);        
	myWrite( arg1, arg2, arg3 );
    }else if( (which == SyscallException) && (type == SC_Close) ){
        DEBUG('2',"System Call: %d invoked Close\n", pid);
        myClose( arg1 );
    }else if( (which == SyscallException) && (type == SC_Fork) ){
        DEBUG('2',"System Call: %d invoked Fork\n", pid);
        myFork( arg1 );
    }else if( (which == SyscallException) && (type == SC_Yield) ){
        DEBUG('2',"System Call: %d invoked Yield\n", pid);
        DEBUG('t', "YIELD, initiated by user program. %s myPID: %d parentPID:%d \n", 
                currentThread->getName(), currentThread->space->pcb->PID, 
                currentThread->space->pcb->parentPID );
        currentThread->Yield();
    }else{
	DEBUG('2',"System Call: %d unexcepected user mode exception %d %d\n", pid, which, type);        
        DEBUG('t',"Unexpected user mode exception %d %d\n", which, type);
        ASSERT( FALSE );
    }
    incrementPC();
}
