#include "randomReplacement.h"
#include <ctime>

RandomReplacement::RandomReplacement(){
    initializeRandom( time( NULL ) );
    for( int i = 0; i < NumPhysPages; i++ ){
        queue[i] = -1;
    }
}

RandomReplacement::~RandomReplacement(){}

void RandomReplacement::pageUsed( int pPage ){
    for( int i = 0; i < NumPhysPages; i++ ){
        if( queue[i] == -1 ){
            queue[i] = pPage;
            return;
        }
    }
}

int RandomReplacement::getReplacement(){
    return getRand() % NumPhysPages;
}

void RandomReplacement::clearPage( int pPage ){
    int i;
    for( i = 0; i < NumPhysPages; i++ ){
        if( queue[i] == pPage ) break;
    }
    queue[i] = -1;
}

void RandomReplacement::markPage( int pPage ){}

void RandomReplacement::initializeRandom( int seed ){
    randoms[0] = seed;
    for( int i = 1; i < 624; i++ ){
        randoms[i] = 0x6c078965 * ( randoms[i - 1] ^ randoms[i - 1] >> 30 ) + 1;
    }
}

void RandomReplacement::generateRandoms(){
    for( int i = 0; i < 624; i++ ){
        int j = ( randoms[i] >> 31 ) + ( randoms[ (i + 1) % 624 ] >> 1 );
        randoms[i] = randoms[(i + 397) % 624] ^ ( j >> 1 );
        if( j % 2 ) randoms[i] = randoms[i] ^ 0x9908b0df;
    }
}

int RandomReplacement::getRand(){
    if( randIdx == 0 ) generateRandoms();

    int rand = randoms[randIdx];
    rand = rand ^ ( rand >> 11 );
    rand = rand ^ ( ( rand << 7 ) & 0x9d2c5680 );
    rand = rand ^ ( ( rand << 15 ) & 0xefc60000 );
    rand = rand ^ ( rand >> 18 );

    randIdx = ( randIdx + 1 ) % 624;
    return rand;
}

