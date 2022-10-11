#!/bin/sh

mpicc -o ping_noblock ping_noblock.c
sbatch -N 2 -n 2 sub_noblock.sh
