#include <unistd.h>
#include <limits.h>

int main(){
    for( int i = 0; i < INT_MAX; i += 2 ){
        i--;
        int pid = fork();
        if( pid ){
            int ret = execl( "time_exec_in", (char *) 0 );
        }
    }
}
