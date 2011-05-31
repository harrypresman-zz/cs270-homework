#include "syscall.h"

void test_function2()
{
  int i=0;
  
//  OpenFileId OutFid, InFid;

  Write("# FORKED 2-1 !!\n", 16, ConsoleOutput);
  Write("# FORKED 2-2 !!\n", 16, ConsoleOutput);
  
  /* maintain this function in memory */
  for( i=0 ; i < 10 ; ++i ) Yield();
  Exit(0);
  
}

void test_function3()
{

  Write("# FORKED 3-1 !!\n", 16, ConsoleOutput);
  Write("# FORKED 3-2 !!\n", 16, ConsoleOutput);
  Yield();
  Exit(0);
}

//char temp[2500];
int
main()
{
   
    OpenFileId OutFid, InFid;
    //int ret = 335;
    int size = 9;
    //int stub = 3;
    char buffer[20];
    
    InFid = Open("out");    
    OutFid = Open("out1");
    
    Write("Test1: First write!\n", 253, ConsoleOutput);
    
    Fork(test_function2);
    
    Write("This is Test1\n", 14, OutFid);
    
    Read(buffer, size, InFid);
    Write(buffer, size, OutFid);
    Yield(); 

    Write("Second write!\n", 14, OutFid);

	Fork(test_function3);

    Yield();
    Write("Third write!\n", 14, OutFid);
    Close(InFid);
    Close(OutFid);
    
	Exec("test/mix_test2");  
    Exit(0);  
/* following code is not reached */
/* it is put here just to consume some memory space*/
  Write("Something's WRONG !!\n", 22, ConsoleOutput);
  Write("Something's WRONG !!\n", 22, ConsoleOutput);
  Write("Something's WRONG !!\n", 22, ConsoleOutput);
  Write("Something's WRONG !!\n", 22, ConsoleOutput);
  Write("Something's WRONG !!\n", 22, ConsoleOutput);
  Write("Something's WRONG !!\n", 22, ConsoleOutput);
  Write("Something's WRONG !!\n", 22, ConsoleOutput);
  Write("Something's WRONG !!\n", 22, ConsoleOutput);
  Write("Something's WRONG !!\n", 22, ConsoleOutput);
  Write("Something's WRONG !!\n", 22, ConsoleOutput);
  Write("Something's WRONG !!\n", 22, ConsoleOutput);
  Write("Something's WRONG !!\n", 22, ConsoleOutput);
  Write("Something's WRONG !!\n", 22, ConsoleOutput);
  Write("Something's WRONG !!\n", 22, ConsoleOutput);
  Write("Something's WRONG !!\n", 22, ConsoleOutput);
  Write("Something's WRONG !!\n", 22, ConsoleOutput);
  Write("Something's WRONG !!\n", 22, ConsoleOutput);
  Write("Something's WRONG !!\n", 22, ConsoleOutput);
  Write("Something's WRONG !!\n", 22, ConsoleOutput);

}
