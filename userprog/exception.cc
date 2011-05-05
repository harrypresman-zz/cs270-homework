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

    char* str = new char[ strlen( machine->mainMemory + pAddr ) ]; // create
    strcpy( str, machine->mainMemory + pAddr ); // we need a char* from mem

    return str;
}

void putString( char* str, int vAddr ){
    int pAddr;
    currentThread->space->Translate( vAddr, &pAddr );

    strcpy( machine->mainMemory + pAddr, str );
}

void getMem( char* buffer, int vAddr, int size ){
    int pAddr;
    currentThread->space->Translate( vAddr, &pAddr ); 
    memcpy( buffer, machine->mainMemory + pAddr, size );
}

void putMem( char* buffer, int vAddr, int size ){
    int pAddr;
    currentThread->space->Translate( vAddr, &pAddr ); 
    memcpy( machine->mainMemory + pAddr, buffer, size ); 
}

void myExit(int exitStatus){
    printf( "EXIT, initiated by user program %s pid:%d parentPID:%d.\n", 
            currentThread->getName(), currentThread->space->pcb->PID, 
            currentThread->space->pcb->parentPID );
    procMgr->setExitStatus(currentThread->space->pcb->PID, exitStatus);
    delete currentThread->space;
    currentThread->Finish();

    machine->WriteRegister(2, exitStatus);
}

void myExec( int vAddr ){
    char* file = getString( vAddr );

    OpenFile* executable = fileSystem->Open( file );

    if( executable == NULL ){
        printf( "Unable to open file %s\n", file );
        return;
    }
    Thread* forkedThread;
    forkedThread = new Thread( "Exec Thread" );
    AddrSpace* space = new AddrSpace( executable );
    forkedThread->space = space;

    forkedThread->space->pcb->parentPID =  currentThread->space->pcb->PID;
    forkedThread->space->pcb->thread = forkedThread;
    delete file;
    delete executable;			// close file

    forkedThread->Fork( execBridge, 0 );
    printf( "EXEC, initiated by user program. %s myPID: %d parentPID:%d \n", 
            currentThread->getName(), currentThread->space->pcb->PID, 
            currentThread->space->pcb->parentPID );
    currentThread->Yield();

    machine->WriteRegister(2, forkedThread->space->pcb->PID);
}

void execBridge( int newPC ){
    currentThread->space->InitRegisters();		// set the initial register values
    currentThread->space->RestoreState();		// load page table register

    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
    // the address space exits
    // by doing the syscall "exit"printf( "EXEC, initiated
    // by user program %s.\n", currentThread->getName() );
}

void myFork( int newPC ){
    // fork kernel thread
    Thread * forkedThread = new Thread("ForkedThread");
    forkedThread->space = new AddrSpace;
    currentThread->space->CopyAddrSpace( forkedThread->space );
    int newPID = procMgr->getPID();
    forkedThread->space->pcb = new PCB(newPID, currentThread->space->pcb->PID,
            forkedThread);
    procMgr->storePCB( forkedThread->space->pcb );
    printf( "FORK, initiated by user program %s. pid:%d parentPID:%d \n", 
            currentThread->getName(), currentThread->space->pcb->PID, 
            currentThread->space->pcb->parentPID );
    forkedThread->Fork( forkBridge, newPC );
    currentThread->Yield();
    printf("returned from fork\n");
    machine->WriteRegister(2, newPID);
}

void forkBridge( int newPC ){
    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();

    printf( "FORK, forked thread %s.\n", currentThread->getName() );
    jumpPC(newPC);

    machine->Run();
}

/*
 * FILESYSTEM CALLS
 */

void myCreate( int vAddr ){
    printf( "CREATE, initiated by user program %s.\n", 
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
    printf( "OPEN %s, initiated by user program %s, assigned FD %d.\n", 
            fileName, currentThread->getName(), userOpenFile->fd );
}

void myClose( int fd ){
    printf( "CLOSE FD %d, initiated by user program %s.\n",
            fd, currentThread->getName() );
    currentThread->space->pcb->closeFile( fd );
    procMgr->closeFile( fd );
}

//LEFTOFF 
int myRead( int vAddr, int size, OpenFileId fd ){
//    printf( "READ, initiated by user program %s.\n", 
//            currentThread->getName() );
    if( fd == ConsoleInput ){
        diskBuffer[0] = getchar(); 
        putMem( diskBuffer, vAddr, 1 );
        machine->WriteRegister( 2, 1 );
    }else{
        // LEFT OFF
        UserOpenFile* file = currentThread->space->pcb->getOpenFile( fd );
        int pos = file->position;
        int numRead = 0;
        int totNumRead = 0;
        if( size > PageSize ){
            int numPages = size/PageSize;
            while( size > 0 ){
                int pSize = (size > PageSize) ? PageSize : size % PageSize;
                numRead = file->sysOpenFile->openFile->ReadAt( diskBuffer, pSize, pos );
                totNumRead += numRead;
                char buffer[numRead];
                memcpy( buffer, diskBuffer, numRead );
                putMem( buffer, vAddr, numRead );
                pos += pSize;
                size -= pSize;
                vAddr += pSize;
            }
            numRead = totNumRead;
            file->position = pos;
        }else{
            int numRead = file->sysOpenFile->openFile->ReadAt( diskBuffer, size, pos );
            char buffer[numRead + 1];
            memcpy( buffer, diskBuffer, numRead );
            buffer[numRead] = '\0';
            putString( buffer, vAddr );
            file->position += size;
            //        printf( "Num read = %d\n", numRead );
        }
        machine->WriteRegister( 2, numRead );
    }
    return 0;
}

//LEFTOFF
int myWrite( int vAddr, int size, OpenFileId fd ){
//    printf( "WRITE, initiated by user program %s.\n", 
//            currentThread->getName() );
    char* buffer = getString( vAddr );
    // ConsoleOutput is not assigned yet
    if( fd == ConsoleOutput ){
        printf( "%s", buffer );
    }else{
        // TODO
        UserOpenFile* file = currentThread->space->pcb->getOpenFile( fd );
        int pos = file->position;
        file->sysOpenFile->openFile->WriteAt( buffer, size, pos );
        file->position += size;
    }
    return 0;
}

//LEFTOFF not tested not right
int userReadWrite( int vAddr, OpenFile* file, int size, int fileAddr, int opType ){
    //TODO helper
    int copySize = PageSize; // diskBuffer size
    while( size > 0 ){
        int pAddr;

        if( size < PageSize ) copySize = size;

        // do we have an openfile at this point? 
        file->ReadAt( diskBuffer, copySize, fileAddr );

        // translate should return the offset in bytes from mainMemory
        bool successful = currentThread->space->Translate( vAddr, &pAddr );

        if( ! successful ) return -1;

        if( opType == READ_OP ){
            bcopy( diskBuffer, machine->mainMemory + pAddr, copySize );
        }else if( opType == WRITE_OP ){ //TODO this is wrong, but think this is the idea of helper	    
            bcopy( machine->mainMemory + pAddr,  diskBuffer, copySize );
        }
        size -= PageSize;
        vAddr += copySize;
        fileAddr += copySize;
    }
    return 1;
    return 0;
}

void ExceptionHandler( ExceptionType which ){
    int type = machine->ReadRegister( 2 );

    int arg1 = machine->ReadRegister( 4 );
    int arg2 = machine->ReadRegister( 5 );
    int arg3 = machine->ReadRegister( 6 );
    int arg4 = machine->ReadRegister( 7 );

    if( (which == SyscallException) && (type == SC_Halt) ){
        printf( "Shutdown, initiated by user program %s.\n", 
                currentThread->getName() );
        interrupt->Halt();
    }else if( (which == SyscallException) && (type == SC_Exit) ){
        myExit( arg1 );
    }else if( (which == SyscallException) && (type == SC_Exec) ){
        myExec( arg1 );
    }else if( (which == SyscallException) && (type == SC_Join) ){
        printf( "JOIN, initiated by user program %s. Waiting on pid:%d \n", 
                currentThread->getName(), machine->ReadRegister(4) );
        procMgr->join( arg1 );
    }else if( (which == SyscallException) && (type == SC_Create) ){
        myCreate( arg1 );
    }else if( (which == SyscallException) && (type == SC_Open) ){
        myOpen( arg1 );
    }else if( (which == SyscallException) && (type == SC_Read) ){
        myRead( arg1, arg2, arg3 );
    }else if( (which == SyscallException) && (type == SC_Write) ){
        //LEFTOFF convert arg1 into a char*? 
        //Is this where we should utilize the helper function?
        myWrite( arg1, arg2, arg3 );
    }else if( (which == SyscallException) && (type == SC_Close) ){
        myClose( arg1 );
    }else if( (which == SyscallException) && (type == SC_Fork) ){
        myFork( arg1 );
    }else if( (which == SyscallException) && (type == SC_Yield) ){
        printf( "YIELD, initiated by user program. %s myPID: %d parentPID:%d \n", 
                currentThread->getName(), currentThread->space->pcb->PID, 
                currentThread->space->pcb->parentPID );
        currentThread->Yield();
    }else{
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT( FALSE );
    }
    incrementPC();
}
