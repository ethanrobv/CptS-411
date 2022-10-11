#!/bin/sh

mpicc -o naive_all_reduce naive_all_reduce.c
sbatch -N 2 -n 8 sub_naive.sh
