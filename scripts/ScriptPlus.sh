#!/bin/bash

tl='|'
for ((i = 0; $i < $2; i++)); do
    tl=$(echo "$tl-")
done
line=$(ls "$1")

for word in $line; do
    word=${word%,}
    echo "$tl$word%,"
    if [ "$2" -gt 0 ] && [ -d "${1}/${word}" ]; then
        nextlev=$(($2 + 2))
        bash $0 $1/$word $nextlev
    fi
done
