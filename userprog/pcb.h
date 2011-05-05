/* Process manager
 * This class provides an interface for user programs' PCBs
 * 
 *
 */

#ifndef PCB_H
#define PCB_H

#include "thread.h"
#include "bitmap.h"
#include "userOpenFile.h"

#define MAX_OPEN_FILES 20

class Thread;

class PCB{
    public:        
        PCB( int pid, int parentPid, Thread* t );
        ~PCB();
        UserOpenFile* getOpenFile( char* name, SysOpenFile* file );
        UserOpenFile* getOpenFile( int fd );
        bool addNewOpenFile( UserOpenFile* file );
        void closeFile( int fd );

        int PID;
        int parentPID;
        Thread* thread;
        UserOpenFile** openFileTable;
        BitMap* openFileMap;
};

#endif
