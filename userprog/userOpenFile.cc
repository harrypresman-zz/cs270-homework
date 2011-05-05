#include "userOpenFile.h"


UserOpenFile::UserOpenFile( char * filename, SysOpenFile* file ){
    fileName = filename;
    sysOpenFile = file;
    position = 0;
    fd = file->fd;
}

