#include "sysOpenFile.h"

SysOpenFile::SysOpenFile( OpenFile* file, char* fname, int id ){
    openFile = file;
    fileName = fname;
    fd = id;
    numUsers = 1;
}

