#include "memoryManager.h"
#include "bitmap.h"

MemoryManager::MemoryManager(){
    bitmap = new BitMap( NumTotPages );
}

MemoryManager::~MemoryManager(){
    delete bitmap;
}

int MemoryManager::getPage(){
    return bitmap->Find();
}

void MemoryManager::clearPage( int i ){
    bitmap->Clear(i);
}
    
int MemoryManager::getFreePageCount(){
    return bitmap->NumClear();
}
