#include "processManager.h"
#include "bitmap.h"


ProcessManager::ProcessManager(){
    bitmap = new BitMap(MAX_PROCS);
    pcbTable = new PCB*[MAX_PROCS];
    conditionTable = new Condition*[MAX_PROCS];
    lockTable = new Lock*[MAX_PROCS];
    bitLock = new Lock("bitLock");
    
}

ProcessManager::~ProcessManager(){
    delete bitmap;
    delete pcbTable;//each pcb is cleaned up by addrspace
    //TODO clean up conditions and locks based on bitmap
    delete bitLock;
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
    bitmap->Clear(pid);
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
	conditionTable[pid]->Wait(lockTable[pid]);
	lockTable[pid]->Release(); 
	return returnStatus[pid];
	
}


void ProcessManager::setExitStatus(int pid, int exitStatus){
	printf("Setting exit status %d for pid %d\n",exitStatus,pid);
	returnStatus[pid]= exitStatus;
}

