/* Process manager
 * This class provides an interface for user programs' PCBs
 * 
 *
 */

#ifndef PROC_MANAGER_H
#define PROC_MANAGER_H

#include "bitmap.h"

#define MAX_PROCS 10

class ProcessManager{
    public:

        ProcessManager();
        ~ProcessManager();

        int getPID();
        void clearPID(int i);
        int getFreePIDCount();

    private:

        BitMap* bitmap;
};

#endif
