#include "pcb.h"
#include "system.h"

PCB::PCB( int pid, int parentPid, Thread* t ){
    PID = pid;
    parentPID = parentPid;
    thread = t;
    openFileTable = new UserOpenFile*[MAX_OPEN_FILES];
    openFileMap = new BitMap( MAX_OPEN_FILES );
}

PCB::~PCB(){ 
    procMgr->clearPID( PID );
}

UserOpenFile* PCB::getOpenFile( char* name, SysOpenFile* file ){
    UserOpenFile* openFile;
    if( openFileMap->Test( file->fd )){ // file open already
        openFile = openFileTable[ file->fd ];
    }else{
        openFileMap->Mark( file->fd );
        openFile = new UserOpenFile( name, file );
        openFileTable[ file->fd ] = openFile;
    }
    return openFile;
}

UserOpenFile* PCB::getOpenFile( int fd ){
    if( openFileMap->Test( fd )){ // file open already
        return openFileTable[ fd ];
    }else{
        return NULL;
    }
}

bool PCB::addNewOpenFile( UserOpenFile* file ){
    if( openFileMap->Test( file->fd ) ) return false;
    else{
        openFileMap->Mark( file->fd );
        return true;
    }
}

void PCB::closeFile( int fd ){
    openFileMap->Clear( fd );
    delete openFileTable[ fd ];
    openFileTable[ fd ] = NULL;
}

