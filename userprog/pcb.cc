#include "pcb.h"
#include "system.h"

PCB::PCB( int pid, int parentPid, Thread* t ){
  PID = pid;
  parentPID = parentPid;
  thread= t;
}
PCB::~PCB(){ 
 procMgr->clearPID(PID);
}

