#!/bin/sh

mpicc -o my_all_reduce my_all_reduce.c
sbatch -N 2 -n 8 sub_myAllReduce.sh
