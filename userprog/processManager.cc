#include "processManager.h"
#include "bitmap.h"
#include "system.h"
#include <string.h>

#define CIN 0
#define COUT 1

ProcessManager::ProcessManager(){
    bitmap = new BitMap(MAX_PROCS);

    pcbTable = new PCB*[MAX_PROCS];
    conditionTable = new Condition*[MAX_PROCS];
    lockTable = new Lock*[MAX_PROCS];

    fsLock = new Lock*[SYS_MAX_OPEN_FILES];
    bitLock = new Lock("bitLock");

    //init fs locks
    for(int i = 0 ; i< SYS_MAX_OPEN_FILES; i++){
      fsLock[i]= new Lock("fsLock");
    }
    
    sysOpenFileTable = new SysOpenFile*[SYS_MAX_OPEN_FILES];
    sysOpenFileMap = new BitMap(SYS_MAX_OPEN_FILES);

    // if we reserve these, we have to instantiate them...
    sysOpenFileMap->Mark( CIN );
    sysOpenFileMap->Mark( COUT );
    
    
}

ProcessManager::~ProcessManager(){
    delete bitmap;
    delete pcbTable;//each pcb is cleaned up by addrspace
    //TODO clean up conditions and locks based on bitmap
    delete bitLock;
    delete fsLock;
    //TODO clean up sysOpenFile Table
    delete sysOpenFileMap;
}

int ProcessManager::getPID(){
    bitLock->Acquire();
    int pid = bitmap->Find();
    bitLock->Release();
    conditionTable[pid] = new Condition("Condition");
    lockTable[pid] = new Lock("Lock");
    return pid;

}

void ProcessManager::clearPID( int pid ){
    //printf("Releasing and signaling for pid : %d \n", pid);
    bitmap->Clear( pid );
    lockTable[pid]->Acquire();
    conditionTable[pid]->Broadcast(lockTable[pid]);
    lockTable[pid]->Release();

    pcbTable[pid] = NULL;
}

int ProcessManager::getFreePIDCount(){
    return bitmap->NumClear();
}

void ProcessManager::storePCB( PCB *p ){
    pcbTable[p->PID] = p;
}

int ProcessManager::join( int pid ) {
    //printf("join, waiting on pid : %d \n", pid);
    lockTable[pid]->Acquire();
    conditionTable[pid]->Wait( lockTable[pid] );
    lockTable[pid]->Release();

    return returnStatus[pid];
}

void ProcessManager::setExitStatus( int pid, int exitStatus ){
    DEBUG('t',"Setting exit status %d for pid %d\n",exitStatus,pid);
    returnStatus[pid] = exitStatus;
}

SysOpenFile* ProcessManager::getOpenFile( char* fileName, OpenFile* openFile ){
    int fd = getFD( fileName );
    if( fd != -1 ){
        return sysOpenFileTable[fd];
    }
    // no open file, just create one!

    //TODO: handle full file table, if find returns -1
    fd = sysOpenFileMap->Find();
    if( fd == -1 ) return NULL;

    SysOpenFile* file = new SysOpenFile( openFile, fileName, fd );
    sysOpenFileTable[fd] = file;

    return file;
}

SysOpenFile* ProcessManager::getOpenFile( int fd ){
    if( sysOpenFileMap->Test( fd ) ){
        return sysOpenFileTable[fd];
    }else{
        return NULL;
    }
}

SysOpenFile* ProcessManager::createNewSysFile( OpenFile* openFile, char* fileName ){
    int index = sysOpenFileMap->Find();

    SysOpenFile* sOF = new SysOpenFile(openFile, fileName, index);
    sysOpenFileTable[index] = sOF;
    return sOF;
}

void ProcessManager::closeFile( int fd ){
    fsLock[fd]->Acquire();
    if(sysOpenFileTable==NULL || sysOpenFileTable[fd]==NULL){
      printf("Null pointer\n");
      return;
    }
    sysOpenFileTable[fd]->numUsers--;
    
    if( sysOpenFileTable[fd]->numUsers == 0 ){
	// TODO: do we want to leave the file, or delete it?
        //fileSystem->Remove( sysOpenFileTable[fd]->fileName );
        //delete sysOpenFileTable[fd]->openFile;
        delete sysOpenFileTable[fd];
	sysOpenFileTable[fd] = NULL;
        sysOpenFileMap->Clear(fd);    
    }
    fsLock[fd]->Release();
}

int ProcessManager::getFD( char* name ){
    /// skip CIN and COUT
    for( int i = 2; i < SYS_MAX_OPEN_FILES; i++ ){
        if( sysOpenFileMap->Test( i ) ){
            // we have a file in this spot of the table
            if ( strcmp( sysOpenFileTable[i]->fileName, name ) == 0 ){
                // we have a match 
                return i;
            }
        }
    }
    return -1;
}
