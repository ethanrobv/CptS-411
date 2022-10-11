#!/bin/sh

#Usage: 'sbatch -N <number_of_nodes> -n <number_of_procs> <path>/sub.sh'

#SBATCH --time=00:20:00

#Change path to executable you want to run
mpirun ~/hello_world
