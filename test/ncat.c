#include "syscall.h"

int main()
{
    OpenFileId input = ConsoleInput;
    OpenFileId output = ConsoleOutput;
    OpenFileId openfileid;
    char ch, buffer[60];
    char *prompt={"Source File:"};
    //char prompt[2];
    char filebuffer[200];
    int i;
    int readnum;

    //    prompt[0] = '-';
    //    prompt[1] = '-';

    Write(prompt, 12, output);

    i = 0;

    do {

        Read(&buffer[i], 1, input); 

    } while( buffer[i++] != '\n' );

    buffer[--i] = '\0';

    if( i > 0 ) {
        openfileid = Open(buffer);
        while(1) {
            readnum = Read(filebuffer, 100, openfileid);
            if(readnum==0) break;
            Write(filebuffer, readnum, output);
        }
    }
    Close(openfileid);
}

