#!/bin/sh
#usage: 'sbatch -N <numberofnodes> -n <number_of_processes> <path>/sub.sh'

#SBATCH --time=00:03:00

mpirun ./game_of_life $1 $2