// directory.cc 
//	Routines to manage a directory of file names.
//
//	The directory is a table of fixed length entries; each
//	entry represents a single file, and contains the file name,
//	and the location of the file header on disk.  The fixed size
//	of each directory entry means that we have the restriction
//	of a fixed maximum size for file names.
//
//	The constructor initializes an empty directory of a certain size;
//	we use ReadFrom/WriteBack to fetch the contents of the directory
//	from disk, and to write back any modifications back to disk.
//
//	Also, this implementation has the restriction that the size
//	of the directory cannot expand.  In other words, once all the
//	entries in the directory are used, no more files can be created.
//	Fixing this is one of the parts to the assignment.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "utility.h"
#include "filehdr.h"
#include "directory.h"
#include "system.h"

//----------------------------------------------------------------------
// Directory::Directory
// 	Initialize a directory; initially, the directory is completely
//	empty.  If the disk is being formatted, an empty directory
//	is all we need, but otherwise, we need to call FetchFrom in order
//	to initialize it from disk.
//
//	"size" is the number of entries in the directory
//----------------------------------------------------------------------

Directory::Directory(int size)
{
    table = new DirectoryEntry[size];
    tableSize = size;
    for (int i = 0; i < tableSize; i++)
	table[i].inUse = FALSE;
}

//----------------------------------------------------------------------
// Directory::~Directory
// 	De-allocate directory data structure.
//----------------------------------------------------------------------

Directory::~Directory()
{ 
    delete [] table;
} 

//----------------------------------------------------------------------
// Directory::FetchFrom
// 	Read the contents of the directory from disk.
//
//	"file" -- file containing the directory contents
//----------------------------------------------------------------------

void
Directory::FetchFrom(OpenFile *file)
{
#ifdef FILESYS
    //Get tableSize
    (void) file->ReadAt((char *)&tableSize, sizeof(int), 0);
    DEBUG('f', "Tablesize :%d \n", tableSize);
    DirectoryEntry* newTable = new DirectoryEntry[tableSize];

    for (int i = 0; i < tableSize; i++)
        newTable[i].inUse = FALSE;

    delete table;
    table = newTable;    
    (void) file->ReadAt((char *)table, tableSize * sizeof(DirectoryEntry), sizeof(int));    
#else
    (void) file->ReadAt((char *)table, tableSize * sizeof(DirectoryEntry), 0);
#endif
}

//----------------------------------------------------------------------
// Directory::WriteBack
// 	Write any modifications to the directory back to disk
//
//	"file" -- file to contain the new directory contents
//----------------------------------------------------------------------

void
Directory::WriteBack(OpenFile *file)
{
#ifdef FILESYS
    DEBUG('f',"Writing a table of size: %d\n",tableSize);
    (void) file->WriteAt((char *) &tableSize, sizeof(int), 0);
    (void) file->WriteAt((char *)table, tableSize * sizeof(DirectoryEntry), sizeof(int));
#else
    (void) file->WriteAt((char *)table, tableSize * sizeof(DirectoryEntry), 0);
    
    
#endif
}

//----------------------------------------------------------------------
// Directory::FindIndex
// 	Look up file name in directory, and return its location in the table of
//	directory entries.  Return -1 if the name isn't in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::FindIndex(char *name)
{
    for (int i = 0; i < tableSize; i++)
        if (table[i].inUse && !strncmp(table[i].name, name, FileNameMaxLen))
	    return i;
    return -1;		// name not in directory
}

//----------------------------------------------------------------------
// Directory::Find
// 	Look up file name in directory, and return the disk sector number
//	where the file's header is stored. Return -1 if the name isn't 
//	in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::Find(char *name)
{
    int i = FindIndex(name);

    if (i != -1)
	return table[i].sector;
    return -1;
}

//----------------------------------------------------------------------
// Directory::Add
// 	Add a file into the directory.  Return TRUE if successful;
//	return FALSE if the file name is already in the directory, or if
//	the directory is completely full, and has no more space for
//	additional file names.
//
//	"name" -- the name of the file being added
//	"newSector" -- the disk sector containing the added file's header
//----------------------------------------------------------------------

bool
Directory::Add(char *name, int newSector)
{ 
    if (FindIndex(name) != -1)
	return FALSE;

    for (int i = 0; i < tableSize; i++)
        if (!table[i].inUse) {
        
        	DEBUG('f',"Adding file %s hdr at sector: %d\n",name,newSector);
            table[i].inUse = TRUE;
            strncpy(table[i].name, name, FileNameMaxLen); 
            table[i].sector = newSector;
        return TRUE;
	}

    //we had no space lets allocate more
    int oldTableSize = tableSize;
    tableSize = (int)(oldTableSize * 1.5);
    DEBUG('f',"** Extending table size from %d to %d - For file %s with hdr at sector %d\n", oldTableSize,tableSize,name,newSector);
    #ifdef USER_PROGRAM
     int pid =  ((currentThread->space != NULL && currentThread->space->pcb != NULL) ?  pid = currentThread->space->pcb->PID : 0 );
    #else
     int pid =  0;        
    #endif
    DEBUG('3',"D [%d][%d]: [%d] -> [%d]\n", pid , 1, oldTableSize, tableSize);   
     
    DirectoryEntry *newTable = new DirectoryEntry[tableSize];
    bzero(newTable,sizeof(DirectoryEntry)*tableSize);
    for (int i = 0; i < tableSize; i++){
      if (i < oldTableSize){
	  //copy the old entries
	  newTable[i].inUse = table[i].inUse;
	  newTable[i].sector = table[i].sector;
	  strncpy(newTable[i].name, table[i].name, FileNameMaxLen+1);
      }
      else{
	//mark the new entries as not in use
	newTable[i].inUse = FALSE;
      }
    }
    //remove old and swap
    delete [] table;
    table = newTable;
    //now add
    if (!table[oldTableSize].inUse){
            table[oldTableSize].inUse = TRUE;
            strncpy(table[oldTableSize].name, name, FileNameMaxLen); 
            table[oldTableSize].sector = newSector;     
	    return true;
    }
    //return this->Add(name,newSector);
	
    //should never get here
    return FALSE;	// no space.  Fix when we have extensible files.
}

//----------------------------------------------------------------------
// Directory::Remove
// 	Remove a file name from the directory.  Return TRUE if successful;
//	return FALSE if the file isn't in the directory. 
//
//	"name" -- the file name to be removed
//----------------------------------------------------------------------

bool
Directory::Remove(char *name)
{ 
    int i = FindIndex(name);
    if (i == -1)
	return FALSE; 		// name not in directory
    table[i].inUse = FALSE;
    return TRUE;	
}

//----------------------------------------------------------------------
// Directory::List
// 	List all the file names in the directory. 
//----------------------------------------------------------------------

void
Directory::List()
{
   for (int i = 0; i < tableSize; i++)
	if (table[i].inUse)
	    printf("%s\n", table[i].name);
}

//----------------------------------------------------------------------
// Directory::Print
// 	List all the file names in the directory, their FileHeader locations,
//	and the contents of each file.  For debugging.
//----------------------------------------------------------------------

void
Directory::Print()
{ 
    FileHeader *hdr = new FileHeader;

    printf("Directory contents:\n");
    for (int i = 0; i < tableSize; i++)
	if (table[i].inUse) {
	    printf("Name: %s, Sector: %d\n", table[i].name, table[i].sector);
	    hdr->FetchFrom(table[i].sector);
	    hdr->Print();
	}
    printf("\n");
    delete hdr;
}
