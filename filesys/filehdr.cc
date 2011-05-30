// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "filehdr.h"

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

bool
FileHeader::Allocate(BitMap *freeMap, int fileSize)
{ 
#ifdef FILESYS
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
    if (freeMap->NumClear() < numSectors)
	return FALSE;		// not enough space

    int i = 0;
    int sectorsToAllocate = numSectors;

    
    DEBUG('f', "Allocating %d sectors for a file size of %d. Sector Size:%d. Max Direct:Indirect %d:%d. Sectors/Indirect %d. Max File Size:%d\n",  numSectors, fileSize, SectorSize, NumDirect,NumInDirect,MaxIndirectPointers,MaxFileSize);
    
    while(sectorsToAllocate > 0){
	if (i < NumDirect){
	  dataSectors[i] = freeMap->Find();
	  sectorsToAllocate--;
	  DEBUG('f', "\nAllocating new direct dataSector[%d]:%d\n" , (i),dataSectors[i]);
	  
	}
	else{
	  indirectPointers[i-NumDirect] = new IndirectPointerBlock();
	  indirectSector[i-NumDirect] = freeMap->Find();
	  DEBUG('f', "\nAllocating new indirectPointerBlock[%d]:%d\n" , (i-NumDirect),indirectSector[i-NumDirect]);
	  int sectorsToAddToThisPage = sectorsToAllocate;
	  if (sectorsToAddToThisPage > MaxIndirectPointers)
	    sectorsToAddToThisPage = MaxIndirectPointers;
	  for (int j = 0 ; j < sectorsToAddToThisPage; j++){
	  
	    //DEBUG('f', "Putting a sector to indirectPointer[%d].\n" , (i-NumDirect));
	    int newSec = freeMap->Find();
	    
	    DEBUG('f', "-%d",newSec );
	    indirectPointers[i-NumDirect]->PutSector(newSec);
	    sectorsToAllocate--;
	  }
	  
	}      
     
      i++;
    }
    DEBUG('f', "\nReturning True for filehdr alloc \n" );
	    
    return TRUE;
    
#else
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
    if (freeMap->NumClear() < numSectors)
	return FALSE;		// not enough space

    for (int i = 0; i < numSectors; i++)
	dataSectors[i] = freeMap->Find();
	 DEBUG('f', "\nReturning True for filehdr alloc \n" );
    return TRUE;
#endif
    
    
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void 
FileHeader::Deallocate(BitMap *freeMap)
{


    for (int i = 0; i < numSectors; i++) {
        printf("Delloc %d (%d)\n",numSectors, NumDirect);
        if (numSectors < NumDirect){
            ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
            freeMap->Clear((int) dataSectors[i]);
        }
    }
    #ifdef FILESYS    
    int indirects = divRoundUp((numSectors - NumDirect),PointersPerIndirect);
    	
    for(int i = 0; i < indirects; i++){
        indirectPointers[i-NumDirect]->Deallocate(freeMap);
	    freeMap->Clear(indirectSector[i-NumDirect]);
	 }
	#endif
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
    
#ifdef FILESYS
	char* buf = new char[SectorSize];
	bzero(buf,SectorSize);
	synchDisk->ReadSector(sector, buf);
	bcopy( buf,(char *) this, HdrSize);
	if (numSectors > NumDirect){
        int indirects = divRoundUp((numSectors - NumDirect),PointersPerIndirect);
    	
      	for(int i = 0; i < indirects; i++){
	
		DEBUG('f',"^^FileHdr fetchin IndirectPointer[%d] at sector:%d\n",i,indirectSector[i]);
		if (indirectSector[i] != 0)
			indirectPointers[i] = new IndirectPointerBlock();
			indirectPointers[i]->FetchFrom(indirectSector[i]);
      }
    }
#else
	synchDisk->ReadSector(sector, (char *)this);      
      
#endif
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
    
#ifdef FILESYS
	char* buf = new char[SectorSize];
	bzero(buf, SectorSize);
	int t = HdrSize;
	bcopy((char *) this, buf, HdrSize);
	synchDisk->WriteSector(sector, buf); 
    if (numSectors > NumDirect){

        int indirects = divRoundUp((numSectors - NumDirect),PointersPerIndirect);
    	
      	for(int i = 0; i < indirects; i++){
	
		DEBUG('f',"^^FileHdr Writing back IndirectPointer[%d] at sector:%d\n",i,indirectSector[i]);
		if (indirectSector[i] != 0)
			indirectPointers[i]->WriteBack(indirectSector[i]);
      }
    }
#else       
    synchDisk->WriteSector(sector, (char *)this);      
#endif
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{
  #ifdef FILESYS
    DEBUG('f',"BytoSector :%d : ",offset);
    
    int sector = offset/ SectorSize;
    if (sector < NumDirect){
      DEBUG('f',"Direct Sector offset %d, is at sector: %d\n",sector,dataSectors[sector]);     
      return dataSectors[sector];
    }
    else{
      int relSec = divRoundDown((numSectors - NumDirect),PointersPerIndirect);
      int newOff = offset - SectorSize * (NumDirect);
      
      DEBUG('f',"Indirect Sector offset %d, with newoffset is at: is at %d : \n",relSec,newOff);
      DEBUG('f'," found at sector: %d\n",indirectPointers[relSec]->ByteToSector(newOff));
      return indirectPointers[relSec]->ByteToSector(newOff);
      
    }
  #else  
    return(dataSectors[offset / SectorSize]);
  #endif
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    int i, j, k;
    char *data = new char[SectorSize];

    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
    for (i = 0; i < numSectors; i++)
	printf("%d ", dataSectors[i]);
    printf("\nFile contents:\n");
    for (i = k = 0; i < numSectors; i++) {
	synchDisk->ReadSector(dataSectors[i], data);
        for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
	    if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
		printf("%c", data[j]);
            else
		printf("\\%x", (unsigned char)data[j]);
	}
        printf("\n"); 
    }
    delete [] data;
}

void FileHeader::setNumBytes(int newBytes){
	numBytes= newBytes;
}

bool FileHeader::ExtendFile(BitMap *freeMap, int sectorsToAllocate){
    DEBUG('f',"Extending file by %d sectors\n",sectorsToAllocate);
    sectorsToAllocate -= numSectors;
	if (sectorsToAllocate <= 0)
		return false;
    if (freeMap->NumClear() < sectorsToAllocate)
        return false;

	int i = numSectors;	
	while(sectorsToAllocate > 0){
	    if (i < NumDirect){
	      dataSectors[i] = freeMap->Find();
	      DEBUG('f', "\nAllocating new direct DataSector[%d]:%d\n" , (i),dataSectors[i]);
	      
	      sectorsToAllocate--;
	      numSectors++;
	    }
	    else{
	      indirectPointers[i-NumDirect] = new IndirectPointerBlock();
	      DEBUG('f', "\nAllocating new indirectPointerBlock[%d]:" , (i-NumDirect));
	      int sectorsToAddToThisPage = sectorsToAllocate;
	      if (sectorsToAddToThisPage > MaxIndirectPointers)
		    sectorsToAddToThisPage = MaxIndirectPointers;
	      for (int j = 0 ; j < sectorsToAddToThisPage; j++){
	      
		    DEBUG('f', "." );
		    //DEBUG('f', "Putting a sector to indirectPointer[%d].\n" , (i-NumDirect));
		    int newSec = freeMap->Find();
		    indirectPointers[i-NumDirect]->PutSector(newSec);
		    sectorsToAllocate--;
		    numSectors++;
	      }
	    }
      
	}   
    return true;
}
