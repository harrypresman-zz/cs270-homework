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

        int PID;
        int parentPID;
        Thread* thread;
        UserOpenFile** openFiles;
        int numOpenFiles;
        //BitMap openFileMap;
        int addNewOpenFile(UserOpenFile *o);
        
        
};

#endif
