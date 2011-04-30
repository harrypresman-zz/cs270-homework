#include "processManager.h"
#include "bitmap.h"


ProcessManager::ProcessManager(){
    bitmap = new BitMap(MAX_PROCS);
}

ProcessManager::~ProcessManager(){
    delete bitmap;
}

int ProcessManager::getPID(){
    return bitmap->Find();
}

void ProcessManager::clearPID(int i){
    bitmap->Clear(i);
}
    
int ProcessManager::getFreePIDCount(){
    return bitmap->NumClear();
}
