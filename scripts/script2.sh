#!/bin/bash

if [ -d dirs ]; then
    rm -r dirs
fi

num=$1
x=1

mkdir dirs
cd dirs
for ((x = 1; x <= num; x++)); do
    mkdir "$x"
    cd "$x"
    y=1
    for ((y = 1; y <= num; y++)); do
        mkdir $x$y
        echo "$x$y" >$x$y/$x$y
        y=$(($y + 1))
    done
    cd ..
    x=$(($x + 1))
done
