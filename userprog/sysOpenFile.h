#ifndef SYS_OPEN_FILE_H
#define SYS_OPEN_FILE_H

class SysOpenFile {
    OpenFileId* openFile;
    FileID fd;
    char* fileName;
    int numUsers;
};

#endif
