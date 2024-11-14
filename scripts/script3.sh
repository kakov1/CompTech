#! /bin/bash

i=1
while [ $i -lt 5 ]; do
    cd dir$i
    echo "$(rev file$i)" > file$i
    i=$(($i + 1))
done

cd ../../../../

# echo $1
mkdir "Directory"
cd "Directory"
for ((i = 1; i < ($1 + 1); i++)); do
    mkdir "Folder $i"
    for ((j = 1; j < ($2 + 1); j++)); do
        touch "Folder $i/File$j"
        for ((k = 1; k < ($2 + 1); k++)); do
            echo "You have come to the File number $j of the Folder number $i >> "Folder $i/File $j""
        done
    done
done
