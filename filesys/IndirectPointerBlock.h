#ifndef INDIRECT_H
#define INDIRECT_H

#include "disk.h"
#include "bitmap.h"


#define MaxIndirectPointers 	((SectorSize - sizeof(int)) / sizeof(int))

class IndirectPointerBlock {
  public:
    IndirectPointerBlock();
    void FetchFrom(int sectorNumber); 	// Initialize IndirectPointerBlock header from disk
    void WriteBack(int sectorNumber); 	// Write modifications to IndirectPointerBlock header
					//  back to disk
    int ByteToSector(int offset);	// Convert a byte offset into the file
					// to the disk sector containing
					// the byte    
    void Deallocate(BitMap *bitMap);  		// De-allocate this file's 
						//  data blocks
    void PutSector(int sector);		// add newly allocated sector to block of block pointers
  private:
    int numSectors;			// Number of data sectors in the file
    int dataSectors[MaxIndirectPointers];		// Disk sector numbers for each data 
					// block in the file  
};

#endif