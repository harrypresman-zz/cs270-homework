
#include "system.h"
#include "IndirectPointerBlock.h"

IndirectPointerBlock::IndirectPointerBlock(){
  numSectors = 0 ; //start with none
}

//----------------------------------------------------------------------
// IndirectPointerBlock::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void IndirectPointerBlock::FetchFrom( int sector ) {
    synchDisk->ReadSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// IndirectPointerBlock::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void IndirectPointerBlock::WriteBack( int sector ) {
    synchDisk->WriteSector(sector, (char *)this); 
}

//----------------------------------------------------------------------
// IndirectPointerBlock::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int IndirectPointerBlock::ByteToSector(int offset) {
    //TODO
    //byte offset relative to the beginning of its IndirectPointerBlock. 
    //if an IndirectPointerBlock covers blocks 60-90 and I pass in an offset of SectorSize*10, 
    //I know that the sector number can be found in the 10th entry. 
    return(dataSectors[offset / SectorSize]);
}


void IndirectPointerBlock::PutSector( int sector ){ 
  dataSectors[numSectors] = sector;
  numSectors++;
}
    
void IndirectPointerBlock::Deallocate(BitMap *bitMap) {
  for( int i=0; i < numSectors ; i ++){
    bitMap->Clear(dataSectors[numSectors]);
    
  }
  numSectors=0;
  
}