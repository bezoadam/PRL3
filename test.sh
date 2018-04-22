#!/bin/bash
tree=$1
size=${#1}
numproc=$((2*$size-2))

mpic++ --prefix /usr/local/share/OpenMPI -o pro pro.cpp
mpirun --prefix /usr/local/share/OpenMPI -np $numproc pro $tree
rm -f pro
