#ifndef SWAP_MANAGER_H
#define SWAP_MANAGER_H

#include "openfile.h"
#include "bitmap.h"

#define SWAP_SIZE 512

class SwapManager{
    public:
        SwapManager();
        ~SwapManager();

        int allocPage();
        bool writePage( int sPage, char* buffer, int size, int offset );
        void swap( int pPageNum, int sPageNum );
        void copy( int fromPage, int toPage );
        void clearPage( int sPage );
        int getFreePageCount();

    private:
        OpenFile* swapFile;
        BitMap* swapMap;
};


#endif
