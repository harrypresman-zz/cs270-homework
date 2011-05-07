
int main(){
    int i, pid;
    for( i = 0; i < INT_MAX; i += 2 ){
        i--;
        Exec( "time/time_exec_in" );
        Yield();
    }
}
