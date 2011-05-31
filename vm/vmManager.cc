#include "vmManager.h"
#include "swapManager.h"
#include "system.h"
#include "fifo.h"
#include "lru.h"
#include "randomReplacement.h"

VMManager::VMManager(){
    swapMgr = new SwapManager();
//    policy = new FIFO();
//    policy = new LRU();
    policy = new RandomReplacement();

    // no pages in physical memory
    for( int i = 0; i < NumPhysPages; i++ ){
        activePageMap[i] = -1;
    }
    activePages = { 0 };
}

VMManager::~VMManager(){
    delete swapMgr;
    delete policy;
}

// allocates a swap page
int VMManager::getPage(){
    return swapMgr->allocPage();
}

// writes to swap
bool VMManager::writePage( int pPage, char* buffer, int size, int offset){
    swapMgr->writePage( pPage, buffer, size, offset );
}

// swaps a swap page into physical memory, clearing room and writing 
// victim to swap if necessary.
int VMManager::swap( TranslationEntry* newPage ){
    // get page number to load
    int pPage;
    int sPage = newPage->physicalPage;
    int oldPage = -1;
	#ifdef USER_PROGRAM
	int pid =  ((currentThread->space->pcb != NULL) ?  pid = currentThread->space->pcb->PID : 0 );
	#else
	int pid =  0;        
	#endif
    if( memMgr->getFreePageCount() > 0 ){
        pPage = memMgr->getPage();
        policy->pageUsed( pPage );
    }else{
        pPage = policy->getReplacement();
        oldPage = activePages[ pPage ]->physicalPage;
        // write old page if necessary
        // TODO: reset use bit??
        activePages[ pPage ]->use = false;
        if( activePages[ pPage ]->dirty ){
            activePages[ pPage ]->dirty = false;
       		DEBUG('3',"S [%d]: [%d]\n", pid, pPage);

            DEBUG( 'v', "Old page %d is dirty, writing to swap!\n", oldPage );
            bcopy( machine->mainMemory + pPage * PageSize, diskBuffer, PageSize );
            swapMgr->writePage( activePageMap[ pPage ], diskBuffer, PageSize, 0 );
        } else{
	        DEBUG('3',"E [%d]: [%d]\n", pid, pPage);
        }        
    }
    

    // swap new page in

    DEBUG( 'v', "Swapping out page %d, swapping in swap page %d to physical page %d\n",
           oldPage, sPage, pPage );
    swapMgr->swap( pPage, sPage );
    activePageMap[ pPage ] = sPage;
    activePages[ pPage ] = newPage;
    
	DEBUG('3',"L [%d]: [%d] -> [%d]\n", pid, oldPage, pPage);
    return pPage;
}

void VMManager::copy( int fromPage, int toPage ){
    swapMgr->copy( fromPage, toPage );
}

// translate a swap page into a physical page and swaps a non-active page in.
int VMManager::getPhysicalPage( TranslationEntry* page ){
    int sPage = page->physicalPage;
    for( int i = 0; i < NumPhysPages; i++ ){
        if( activePageMap[i] == sPage ) return i;
    }
    // the page is not active, so swap
    return swap( page );
}

// remove page from active map, clear in swap mgr
void VMManager::clearPage( int sPage ){
    int pPage;
    DEBUG( '3', "Clearing page %d\n", pPage );
    for( pPage = 0; pPage < NumPhysPages; pPage++ ){
        if( activePageMap[ pPage ] == sPage ){
            activePageMap[ pPage ] = -1;
            activePages[ pPage ] = 0;
            break;
        }
    }
    memMgr->clearPage( pPage );
    swapMgr->clearPage( sPage );
    policy->clearPage( pPage );
}

// return the number of free swap sectors
int VMManager::getFreePageCount(){
    return swapMgr->getFreePageCount();
}

void VMManager::markPage( int pPage ){
    policy->markPage( pPage );
}

