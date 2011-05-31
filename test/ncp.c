#include "syscall.h"

int main(int argc, char **argv)
{
    char inputfile[60], outputfile[60];

    OpenFileId input = ConsoleInput;
    OpenFileId output = ConsoleOutput;
    OpenFileId openfileid;
    char buffer[60];
    OpenFileId inputfileid, outputfileid;
    char *prompt1={"Source File:"};
    char *prompt2={"Output File:"};
    char filebuffer[200];
    int i, j;
    int readnum;

    Write("push page\n", 12, output);

    Write(prompt1, 12, output);

    i = 0;

    do {

        Read(&inputfile[i], 1, input); 

    } while( inputfile[i++] != '\n' );

    inputfile[--i] = '\0';

    Write(prompt2, 12, output);
    j = 0;

    do {

        Read(&outputfile[j], 1, input); 

    } while( outputfile[j++] != '\n' );

    outputfile[--j] = '\0';


    //    if(argc != 3) {

    //        cout<<"Wrong parameter format!";
    //        return;
    //    }

    //    strcpy(inputfile = *argv;
    //    outputfile = *(argv++);

    if( i > 0 && j>0 ) {
        inputfileid = Open(inputfile);
        Create(outputfile);
        outputfileid = Open(outputfile);
        while(1) {
            readnum = Read(filebuffer, 200, inputfileid);
            if(readnum==0) break;
            Write(filebuffer, readnum, outputfileid);
        }
    }
    Close(inputfileid);
    Close(outputfileid);	
}

