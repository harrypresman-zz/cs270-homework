#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <sys/wait.h>

int main(){
    printf("%d\n", INT_MAX );
    for( int i = 0; i < 10; i += 2 ){
        i--;
        int pid = fork();
        if( !pid ){
            int ret = execle( "time_exec_in", (char *) 0 );
        }else{
            int status;
            waitpid( pid, &status, 0 );
        }
    }
}
