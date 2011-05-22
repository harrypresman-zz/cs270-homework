#ifndef LRU_H
#define LRU_H

#include "iReplacement.h"
#include "machine.h"

class LRU : public IReplacement {
    public:
        LRU();
        ~LRU();
        virtual void pageUsed( int pPage );
        virtual int getReplacement();
        virtual void clearPage( int pPage );
        virtual void markPage( int pPage );

    private:
        int queue[NumPhysPages];
};

#endif
