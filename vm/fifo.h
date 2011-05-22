#ifndef FIFO_H
#define FIFO_H

#include "iReplacement.h"
#include "machine.h"

class FIFO : public IReplacement {
    public:
        FIFO();
        ~FIFO();
        virtual void pageUsed( int pPage );
        virtual int getReplacement();
        virtual void clearPage( int pPage );
        virtual void markPage( int pPage );

    private:
        int queue[NumPhysPages];
};

#endif
