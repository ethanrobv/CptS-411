#!/bin/sh
#usage: 'sbatch -N <numberofnodes> -n <number_of_processes> <path>/sub.sh'

#SBATCH --time=00:20:00

mpirun ./ping_noblock
