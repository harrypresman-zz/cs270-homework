#include "lru.h"

LRU::LRU(){ 
    for( int i = 0; i < NumPhysPages; i++ ){
        queue[i] = -1;
    }
}

LRU::~LRU(){}

void LRU::pageUsed( int pPage ){
    for( int i = 0; i < NumPhysPages; i++ ){
        if( queue[i] == -1 ){
            queue[i] = pPage;
            return;
        }
    }
}

int LRU::getReplacement(){
    int page = queue[0];
    for( int i = 1; i < NumPhysPages; i++ ){
        queue[ i - 1] = queue[i];
    }
    queue[ NumPhysPages - 1 ] = page;
    return page;
}

void LRU::clearPage( int pPage ){
    bool copy = false;
    int i;
    for( i = 0; i < NumPhysPages; i++ ){
        if( queue[i] == pPage ) break;
    }
    for( ; i < NumPhysPages - 1; i++ ){
        if( queue[i] == -1 ) break;
        queue[i] = queue[ i + 1 ];
    }
    queue[i] = -1;
}

void LRU::markPage( int pPage ){
    int i;
    int page;
    for( i = 0; i < NumPhysPages; i++ ){
        if( queue[i] == pPage ){ 
            page = queue[i];
            break;
        }
    }
    for( ; i < NumPhysPages - 1; i++ ){
        if( queue[i] == -1 ) break;
        queue[i] = queue[ i + 1 ];
    }
    queue[i] = page;
}
