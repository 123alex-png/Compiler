#!/bin/sh
flex lexical.l
gcc main.c lex.yy.c -lfl -o scanner
./scanner ../Test/my_test.cmm