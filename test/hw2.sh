#!/bin/bash

# This script is part of my automate grading test script, 
# it will compile your nachos program and execute the specified test programs
#
# Usage: hw2 < your nachos code dir > [ list of test programs ]
# Example: ./hw2.sh ~/wei/code test1 test2 test3

cd $1
make
cd $1/userprog

for i in $(seq 2 $#)
do
    echo Now testing ${!i}
    ./nachos -d 2 -x $1/test/${!i}
done