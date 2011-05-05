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
#include "sysOpenFile.h"

#define MAX_PROCS 10
#define SYS_MAX_OPEN_FILES 20

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
        SysOpenFile* getOpenFile( char* fileName );
        SysOpenFile* createNewSysFile( OpenFile* openFile, char* fileName);
        void closeFile( int id);

    private:
		PCB** pcbTable; //the array of PCBS use bitmap to know what is alive
		Condition** conditionTable; //each PCB gets a condition
		Lock** lockTable; //lock required to acquire condition
		Lock* bitLock;
		Lock** fsLock; //locks for openfile modification
    BitMap* bitmap; 
    int returnStatus[MAX_PROCS]; // array of return statuses for threads waiting on join
    SysOpenFile** sysOpenFileTable;
    BitMap* sysOpenFileMap;
};

#endif
