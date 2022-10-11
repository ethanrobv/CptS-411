#!/bin/sh

mpicc -o ping_block ping_block.c
sbatch -N 2 -n 2 sub_block.sh
