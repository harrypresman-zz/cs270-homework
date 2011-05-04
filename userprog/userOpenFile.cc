#include "userOpenFile.h"


UserOpenFile::UserOpenFile(char * fn, SysOpenFile* sOF){
  fileName = fn;
  sysOpenFile = sOF;
  position = 0;
}

