/* test5.c */
/* --------- testing system calls : Exit, Exec, Fork */
/* --------- size of the program : 12 pages			      */

#include "syscall.h"

void test5_1(){
	Write( "Test5 Fork(): This is test 5 after fork before Exec\n", 60, ConsoleOutput );
	Exec( "test/test5_1" );
    Yield();
    Exit( 0 );
}

int main(){
  int i;

  for( i = 0 ; i < 10 ; ++i ) {
  	Fork( test5_1 );
  	Yield();
  }
  for( i = 0 ; i < 10 ; ++i ) Exec( "test/test5_2" );
  
  Halt(0);
}

