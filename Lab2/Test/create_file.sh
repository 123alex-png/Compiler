#!/bin/bash
int=1
while(($int<=6))
do
    filename="optional_example"$int".cmm"
    touch ./$filename
    let "int++"
done