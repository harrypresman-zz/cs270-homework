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
#define BadVAddrReg	39	// Stores vaddr on pagefault exception

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

void decrementPC(){
    machine->registers[NextPCReg] = machine->registers[PCReg];
    machine->registers[PCReg] = machine->registers[PrevPCReg];
    machine->registers[PrevPCReg] = 0;
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

    // copy string
    int size = strlen( machine->mainMemory + pAddr ) + 1;
    char* str = new char[ size ];
    strcpy( str, machine->mainMemory + pAddr );
    str[size] = '\0';

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
    IntStatus oldLevel = interrupt->SetLevel( IntOff );
    DEBUG('t', "EXIT, initiated by user program %s pid:%d parentPID:%d.\n", 
            currentThread->getName(), currentThread->space->pcb->PID, 
            currentThread->space->pcb->parentPID );
    int pid = currentThread->space->pcb->PID;
    procMgr->setExitStatus( currentThread->space->pcb->PID, exitStatus );
    
    DEBUG('2', "Process %d exits with %d\n", pid , exitStatus); 
    delete currentThread->space;
    currentThread->Finish();

    machine->WriteRegister( 2, exitStatus );
    interrupt->SetLevel( oldLevel );
}

void myExec( int vAddr ){
    IntStatus oldLevel = interrupt->SetLevel( IntOff );
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


    forkedThread->space->pcb->parentPID = currentThread->space->pcb->PID;
    forkedThread->space->pcb->thread = forkedThread;
    DEBUG('2', "Exec Program: %d loading %s\n", 
            currentThread->space->pcb->PID , file );

    delete file;
    delete executable;			// close file

    
    forkedThread->Fork( execBridge, 0 );
    DEBUG('t', "EXEC, initiated by user program. %s myPID: %d parentPID:%d\n", 
            currentThread->getName(), currentThread->space->pcb->PID, 
            currentThread->space->pcb->parentPID );
    
    machine->WriteRegister( 2, forkedThread->space->pcb->PID );
    interrupt->SetLevel( oldLevel );
    currentThread->Yield();
}

void execBridge( int newPC ){
    currentThread->space->InitRegisters();		// set the initial register values
    currentThread->space->RestoreState();		// load page table register

    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
    // the address space exits
    // by doing the syscall "exit"
}

void myFork( int newPC ){
    IntStatus oldLevel = interrupt->SetLevel( IntOff );
    // fork kernel thread
    Thread * forkedThread = new Thread( "ForkedThread" );
    forkedThread->space = new AddrSpace;
    currentThread->space->CopyAddrSpace( forkedThread->space );
    int newPID = procMgr->getPID();
    forkedThread->space->pcb = new PCB(newPID, currentThread->space->pcb->PID,
            forkedThread);
    procMgr->storePCB( forkedThread->space->pcb );
    DEBUG( 't', "FORK, initiated by user program %s. pid:%d parentPID:%d \n", 
            currentThread->getName(), currentThread->space->pcb->PID, 
            currentThread->space->pcb->parentPID );
    DEBUG( '2', "Process %d  Fork: start at address 0x%x with %d pages memory\n",
            currentThread->space->pcb->PID, newPC, 
            forkedThread->space->numPages );
    forkedThread->Fork( forkBridge, newPC );
    currentThread->Yield();
    DEBUG( 't', "returned from fork\n" );
    
    machine->WriteRegister( 2, newPID );
    interrupt->SetLevel( oldLevel );
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
    IntStatus oldLevel = interrupt->SetLevel( IntOff );
    DEBUG('t',"CREATE, initiated by user program %s.\n", 
            currentThread->getName() );
    char* fileName = getString( vAddr );
    fileSystem->Create( fileName, 0 );
    interrupt->SetLevel( oldLevel );
}

void myOpen( int vAddr ){
    IntStatus oldLevel = interrupt->SetLevel( IntOff );
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
    interrupt->SetLevel( oldLevel );
}

void myClose( int fd ){
    IntStatus oldLevel = interrupt->SetLevel( IntOff );
    DEBUG('t', "CLOSE FD %d, initiated by user program %s.\n",
            fd, currentThread->getName() );
    currentThread->space->pcb->closeFile( fd );
    procMgr->closeFile( fd );
    interrupt->SetLevel( oldLevel );
}

int myRead( int vAddr, int size, OpenFileId fd ){
    IntStatus oldLevel = interrupt->SetLevel( IntOff );
    if( fd == ConsoleInput ){
        //TODO: need page alignment fix - will size ever cross a page boundary?
        for( int i = 0; i < size; i++ ){
            diskBuffer[i] = getchar(); 
        }
        putMemIntoVAddr( diskBuffer, vAddr, size + 1 );
        machine->WriteRegister( 2, size );
    }else{
        UserOpenFile* file = currentThread->space->pcb->getOpenFile( fd );
        int pos = file->position;
        int numRead = 0;
        int totNumRead = 0;
        while( size > 0 ){
            int copySize = PageSize;
            int offset = vAddr % PageSize;
            
            if( size < PageSize ) copySize = size;

            if( copySize + offset >= PageSize ) copySize = PageSize - offset;

            bzero( diskBuffer, PageSize );
            numRead = file->sysOpenFile->openFile->ReadAt( diskBuffer, copySize, pos );
            putMemIntoVAddr( diskBuffer, vAddr, numRead );

            totNumRead += numRead;
            pos += copySize;
            size -= copySize;
            vAddr += copySize;
        }
        file->position += totNumRead;
        machine->WriteRegister( 2, totNumRead );
    }
    interrupt->SetLevel( oldLevel );
    return 0;
}

void myWrite( int vAddr, int size, OpenFileId fd ){
    IntStatus oldLevel = interrupt->SetLevel( IntOff );
    if( fd == ConsoleOutput ){
        char* buffer = new char[ size + 1 ];
        int numWritten = 0;
        while( size > 0 ){
            int copySize = PageSize;
            int offset = vAddr % PageSize;

            if( size < PageSize ) copySize = size;
            if( copySize + offset >= PageSize ) copySize = PageSize - offset;

            getMemFromVAddr( buffer + numWritten, vAddr, copySize );

            numWritten += copySize;
            size -= copySize;
        }
        buffer[numWritten] = 0;
        printf("%s", buffer );
        fflush( stdout );
    }else{
        UserOpenFile* file = currentThread->space->pcb->getOpenFile( fd );
        int pos = file->position;
        int numRead = 0;
        while( size > 0 ){
            int copySize = PageSize;
            int offset = vAddr % PageSize;

            if( size < PageSize ) copySize = size;
            if( copySize + offset >= PageSize ) copySize = PageSize - offset;

            bzero( diskBuffer, PageSize );
            getMemFromVAddr( diskBuffer, vAddr, copySize );
            numRead = file->sysOpenFile->openFile->WriteAt( diskBuffer, 
                    copySize, pos );

            pos += copySize;
            size -= copySize;
            vAddr += copySize;
        }
        file->position = pos;
    }
    interrupt->SetLevel( oldLevel );
}

void myPageFaultHandler( int vAddr ){
    IntStatus oldLevel = interrupt->SetLevel( IntOff );
    int vPageNum = vAddr/PageSize;
    int sPageNum = currentThread->space->pageTable[ vPageNum ].physicalPage;
    TranslationEntry* page = &( currentThread->space->pageTable[ vPageNum ] );
#ifdef VM
    vmMgr->swap( page );
    currentThread->space->pageTable[ vPageNum ].valid = true;
#endif
    interrupt->SetLevel( oldLevel );
}

void ExceptionHandler( ExceptionType which ){
    int type = machine->ReadRegister( 2 );

    int arg1 = machine->ReadRegister( 4 );
    int arg2 = machine->ReadRegister( 5 );
    int arg3 = machine->ReadRegister( 6 );
    //int arg4 = machine->ReadRegister( 7 ); // never used
    int pid = currentThread->space->pcb->PID;
    if( which == PageFaultException ){
        int vAddr = machine->ReadRegister( BadVAddrReg );
        myPageFaultHandler( vAddr );
    }else if( which == SyscallException ){
        if( type == SC_Halt ){
            DEBUG('2',"System Call: %d invoked Halt\n", pid );
            DEBUG('t', "Shutdown, initiated by user program %s.\n", 
                    currentThread->getName() );
            interrupt->Halt();
        }else if( type == SC_Exit ){
            DEBUG('2',"System Call: %d invoked Exit\n", pid );
            myExit( arg1 );
        }else if( type == SC_Exec ){
            DEBUG('2',"System Call: %d invoked Exec\n", pid );
            myExec( arg1 );
        }else if( type == SC_Join ){
            DEBUG('2',"System Call: %d invoked Join\n", pid );
            DEBUG('t', "JOIN, initiated by user program %s. Waiting on pid:%d \n", 
                    currentThread->getName(), arg1 );
            procMgr->join( arg1 );
        }else if( type == SC_Create ){
            DEBUG('2',"System Call: %d invoked Create\n", pid );
            myCreate( arg1 );
        }else if( type == SC_Open ){
            DEBUG('2',"System Call: %d invoked Open\n", pid );
            myOpen( arg1 );
        }else if( type == SC_Read ){
            DEBUG('2',"System Call: %d invoked Read\n", pid );
            myRead( arg1, arg2, arg3 );
        }else if( type == SC_Write ){
            DEBUG('2',"System Call: %d invoked Write\n", pid );
            myWrite( arg1, arg2, arg3 );
        }else if( type == SC_Close ){
            DEBUG('2',"System Call: %d invoked Close\n", pid );
            myClose( arg1 );
        }else if( type == SC_Fork ){
            DEBUG('2',"System Call: %d invoked Fork\n", pid );
            myFork( arg1 );
        }else if( type == SC_Yield ){
            DEBUG('2',"System Call: %d invoked Yield\n", pid );
            DEBUG('t', "YIELD, initiated by user program. %s myPID: %d parentPID:%d \n", 
                    currentThread->getName(), currentThread->space->pcb->PID, 
                    currentThread->space->pcb->parentPID );
            currentThread->Yield();
        }
        incrementPC();
    }else{
        DEBUG('2',"System Call: %d unexpected user mode exception %d %d\n", 
                pid, which, type);
        DEBUG('t',"Unexpected user mode exception %d %d\n", which, type);
        ASSERT( FALSE );
    }
}
