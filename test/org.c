
#include "syscall.h"

int main(){
    OpenFileId Fid;
    int ret =335;
    int size = 9;
    int stub =3;
    char buffer[20];

    Exec("test/test1");
    Exit(ret);
    Yield();

    Create("test");    
    Fid = Open("test");
    Read(buffer, size, Fid);
    Write("First write!", size, Fid);
    Close(Fid);
    Halt(); 
    /* not reached */
}

