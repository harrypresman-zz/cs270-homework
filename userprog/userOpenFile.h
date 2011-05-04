#ifndef USER_OPEN_FILE_H
#define USER_OPEN_FILE_H

#include "sysOpenFile.h"

class UserOpenFile{
  public:
    char* fileName;
    SysOpenFile* sysOpenFile;
    int position;
    int sysOpenFileIndex;
    UserOpenFile(char * fileName, SysOpenFile* sysOpenFile); //set position = 0
};

#endif
