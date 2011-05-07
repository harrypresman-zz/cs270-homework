
int child(){
    Exit();
}

int main(){
    int i, pid;
    for( i = 0; i < 10; i += 2 ){
        i--;
        Fork( child, 0 );
        Yield();
    }
}
