#!/bin/sh

mpicc -o MPI_all_reduce MPI_all_reduce.c
sbatch -N 2 -n 8 sub_MPIAllReduce.sh
