#!/bin/bash
tree=$1
numproc=expr length tree

mpic++ --prefix /usr/local/share/OpenMPI -o pro pro.cpp
mpirun --prefix /usr/local/share/OpenMPI -np $numproc pro tree
rm -f pro
