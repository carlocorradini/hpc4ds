#!/bin/bash

#PBS -l nodes=2:ppn=1
#PBS -q short_cpuQ

# Current working directory
# See https://unix.stackexchange.com/questions/207205/current-directory-in-qsub
cd "$PBS_O_WORKDIR" || exit $?

readonly NUMBER_PROCESSES=2

module load mpich-3.2
mpirun.actual -np "${NUMBER_PROCESSES}" ../navierstokes --simulations=../simulations.json --results=../results
