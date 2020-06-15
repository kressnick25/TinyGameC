#!/bin/bash
gcc -std=gnu99 -Wall -Werror -g ./main.c -o ./out -I ./ZDK/ -L ./ZDK/ -lzdk -lncurses -lm 
