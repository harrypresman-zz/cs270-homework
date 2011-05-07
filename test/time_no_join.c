#include <unistd.h>

int main(){
    for( int i = 0; i < 10; i += 2 ){
        i--;
        int pid = fork();
        if( !pid ){
            int ret = execle( "time_exec_in", (char *) 0 );
        }else{
            int status;
        }
    }
}
