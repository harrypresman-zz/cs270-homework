/* Process manager
 * This class provides an interface for user programs' PCBs
 * 
 *
 */

#ifndef PCB_H
#define PCB_H

#include "thread.h"

class Thread;
class PCB{
    public:        
        PCB( int pid, int parentPid, Thread* t );
        ~PCB();

        int PID;
        int parentPID;
        Thread* thread;
};

#endif
