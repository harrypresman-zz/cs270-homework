#include "pcb.h"
#include "system.h"

PCB::PCB( int pid, int parentPid, Thread* t ){
  PID = pid;
  parentPID = parentPid;
  thread= t;
  openFiles = new UserOpenFile*[MAX_OPEN_FILES];
  numOpenFiles = 0;
//  openFileMap = BitMap(MAX_OPEN_FILES);
}
PCB::~PCB(){ 
 procMgr->clearPID(PID);
}

int PCB::addNewOpenFile( UserOpenFile* o ){

  return -1 ; //already exists
}

