#!/bin/bash
gcc -Wall -g -o k lcm.cpp -std=c++11 -lstdc++ 
./k | tee log.txt
echo '
log.txtにも結果を出力しました。
'
