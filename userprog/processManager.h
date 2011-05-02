/* Process manager
 * This class provides an interface for user programs' PCBs
 * 
 *
 */

#ifndef PROC_MANAGER_H
#define PROC_MANAGER_H

#include "bitmap.h"
#include "pcb.h"
#include "synch.h"

#define MAX_PROCS 10

class ProcessManager{
    public:

        ProcessManager();
        ~ProcessManager();

        int getPID();
        void clearPID(int pid);
        int getFreePIDCount();
        void storePCB(PCB *p);
        int join(int pid);
        void setExitStatus(int pid, int exitStatus);

    private:
		PCB** pcbTable; //the array of PCBS use bitmap to know what is alive
		Condition** conditionTable; //each PCB gets a condition
		Lock** lockTable; //lock required to acquire condition
		Lock* bitLock;
        BitMap* bitmap; 
        int returnStatus[MAX_PROCS]; // array of return statuses for threads waiting on join
};

#endif
