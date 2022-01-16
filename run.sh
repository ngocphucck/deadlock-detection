# !usr/bin/bash

figlet -f slant "Deadlock detection"
gcc banker.c -o banker.o
./banker.o
rm banker.o
