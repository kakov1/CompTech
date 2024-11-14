# !/bin/bash

if [ $# -ge 0 ]; then
  disk="sda"
else
  disk=$1
fi

String=$(iostat -dhx "$disk" 1 2)
echo ${String:-6}
