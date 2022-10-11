#!/bin/sh

mpicc -o game_of_life game_of_life.c
sbatch -N 2 -n 4 sub_GOL.sh