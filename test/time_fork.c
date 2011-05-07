#include <unistd.h>

int child(){}

int main(){
    for( int i = 0; i < 10; i += 2 ){
        i--;
        int pid = fork();
        if( !pid ){
            child();
        }else{
            int status;
        }
    }
}
