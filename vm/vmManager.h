#ifndef VM_MANAGER_H
#define VM_MANAGER_H

#include "swapManager.h"
#include "iReplacement.h"
#include "translate.h"
#include "machine.h"

class VMManager{
    public:
        VMManager();
        ~VMManager();

        int getPage();
        bool writePage( int pPage, char* buffer, int size, int offset );
        int swap( TranslationEntry* page );
        void copy( int fromPage, int toPage );
        int getPhysicalPage( TranslationEntry* page );
        void clearPage( int sPage );
        int getFreePageCount();
        void markPage( int pPage );

    private:
        SwapManager* swapMgr;
        IReplacement* policy;
        int activePageMap[ NumPhysPages ];
        TranslationEntry* activePages[ NumPhysPages ];
};

#endif
