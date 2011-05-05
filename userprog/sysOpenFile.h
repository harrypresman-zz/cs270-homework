#ifndef SYS_OPEN_FILE_H
#define SYS_OPEN_FILE_H

#include "openfile.h"
#include "syscall.h"


class SysOpenFile {
    public:
        OpenFile* openFile;
        OpenFileId fd;
        char* fileName;
        int numUsers;

        SysOpenFile(OpenFile* openFile, char* fileName, int fd); //set numUsers = 1
};

#endif
