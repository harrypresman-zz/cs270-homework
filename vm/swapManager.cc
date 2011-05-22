#include "swapManager.h"
#include "system.h"
#include "machine.h"

SwapManager::SwapManager(){
    fileSystem->Create( "SWAP", SWAP_SIZE * PageSize );
    swapFile = fileSystem->Open( "SWAP" );

    swapMap = new BitMap( SWAP_SIZE );
}

SwapManager::~SwapManager(){
    fileSystem->Remove( "SWAP" );

    delete swapMap;
}

// allocate a new page from swap
int SwapManager::allocPage(){
    int page = swapMap->Find();
    ASSERT( page >= 0 );

    // zero out memory in case this page has garbage remaining
    bzero( diskBuffer, PageSize );
    swapFile->WriteAt( diskBuffer, PageSize, page * PageSize );
    
    return page;
}

// write buffer into swap
bool SwapManager::writePage( int sPage, char* buffer, int size, int offset ){
    swapFile->WriteAt( buffer, size, sPage * PageSize + offset );
}

// copy swap page into physical memory
void SwapManager::swap( int pPageNum, int sPageNum ){
    swapFile->ReadAt( diskBuffer, PageSize, sPageNum * PageSize );
    bcopy( diskBuffer, machine->mainMemory + pPageNum * PageSize, PageSize );    
}

// copy one page in swap to another in swap
void SwapManager::copy( int fromPage, int toPage ){
    swapFile->ReadAt( diskBuffer, PageSize, fromPage * PageSize );
    swapFile->WriteAt( diskBuffer, PageSize, toPage * PageSize );
}

// clear this page in swap
void SwapManager::clearPage( int sPage ){
    bzero( diskBuffer, PageSize );
    swapFile->WriteAt( diskBuffer, PageSize, sPage * PageSize );
    swapMap->Clear( sPage );
}

// return the number of free swap pages
int SwapManager::getFreePageCount(){
    return swapMap->NumClear();
}
