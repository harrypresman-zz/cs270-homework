#include "syscall.h"

int main(int argc, char **argv)
{
    char inputline[60];

    OpenFileId input = ConsoleInput;
    OpenFileId output = ConsoleOutput;
    char buffer[60];
    char *prompt1={"Source line:"};
    char *prompt2={"Output line:"};
    char filebuffer[200];
    int i, j;
    int readnum;

    Write(prompt1, 12, output);

    i = 0;
    do {	
        if (i==60)
            break;
        Read(&inputline[i], 1, input); 
    } while( inputline[i++] != '\n' );

    inputline[--i] = '\0';

    Write(prompt2, 12, output);
    Write(inputline, 60, output);

    Write(prompt1, 12, output);

    i = 0;
    do {	
        if (i==60)
            break;
        Read(&inputline[i], 1, input); 
    } while( inputline[i++] != '\n' );

    inputline[--i] = '\0';

    Write(prompt2, 12, output);
    Write(inputline, 60, output);
}
