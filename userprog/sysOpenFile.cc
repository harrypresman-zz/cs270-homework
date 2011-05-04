#include "sysOpenFile.h"

    
SysOpenFile::SysOpenFile(OpenFile* file, char* fname, int id){

//set numUsers = 1
  openFile = file;
  fileName = fname;
  fd = id;
  numUsers = 1;
}



