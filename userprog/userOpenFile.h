#ifndef USER_OPEN_FILE_H
#define USER_OPEN_FILE_H

#include "sysOpenFile.h"

class UserOpenFile{
    public:
        UserOpenFile(char * fileName, SysOpenFile* sysOpenFile); // set position = 0

        char* fileName;
        SysOpenFile* sysOpenFile;
        int position;
        int fd;
};

#endif
