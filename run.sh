#!/bin/bash
screen -S $1D-L=$2-PP=$3-MAXMCS=$4  ./landau-wang-omp -D $1 -L $2 -PP $3 -MAXMCS $4
