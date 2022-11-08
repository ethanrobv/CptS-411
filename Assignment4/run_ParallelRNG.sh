#!/bin/sh

mpicc -o rng rng.c
sbatch -N 2 -n $6 sub_ParallelRNG.sh $1 $2 $3 $4 $5