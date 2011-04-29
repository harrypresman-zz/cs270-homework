/* Memory manager
 * This class provides an interface for user programs' address space into the machine's memory
 * 
 *
 */

#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "machine.h"
#include "bitmap.h"

#define NumTotPages NumPhysPages


class MemoryManager{
    public:

        MemoryManager();
        ~MemoryManager();

        int getPage();
        void clearPage(int i);
        int getFreePageCount();

    private:

        BitMap* bitmap;
};

#endif
