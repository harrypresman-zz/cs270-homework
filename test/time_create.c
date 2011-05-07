#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(){
    char* file = "/home/aelmore/temp/tempc1";
    creat(file,O_CREAT);
}