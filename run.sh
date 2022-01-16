# !usr/bin/bash

figlet -f slant "Deadlock simulation"
gcc banker.c -o banker.o
./banker.o
rm banker.o
