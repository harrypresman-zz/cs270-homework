#ifndef RANDOM_REPLACEMENT_H
#define RANDOM_REPLACEMENT_H

#include "iReplacement.h"
#include "machine.h"

class RandomReplacement : public IReplacement {
    public:
        RandomReplacement();
        ~RandomReplacement();
        virtual void pageUsed( int pPage );
        virtual int getReplacement();
        virtual void clearPage( int pPage );
        virtual void markPage( int pPage );

    private:
        void initializeRandom( int seed );
        void generateRandoms();
        int getRand();

        int queue[NumPhysPages];
        int randoms[624];
        int randIdx;
};

#endif
