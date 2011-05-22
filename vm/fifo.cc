#include "fifo.h"

FIFO::FIFO(){ 
    for( int i = 0; i < NumPhysPages; i++ ){
        queue[i] = -1;
    }
}

FIFO::~FIFO(){}

void FIFO::pageUsed( int pPage ){
    for( int i = 0; i < NumPhysPages; i++ ){
        if( queue[i] == -1 ){
            queue[i] = pPage;
            return;
        }
    }
}

int FIFO::getReplacement(){
    int page = queue[0];
    for( int i = 1; i < NumPhysPages; i++ ){
        queue[ i - 1] = queue[i];
    }
    queue[ NumPhysPages - 1 ] = page;
    return page;
}

void FIFO::clearPage( int pPage ){
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

void FIFO::markPage( int pPage ){
    // no op
}
