#!/bin/bash

#PBS -l nodes=1
#PBS -q short_cpuQ

readonly NUMBER_PROCESSES=2

module load mpich-3.2
mpirun.actual -np "${NUMBER_PROCESSES}" ../navierstokes --simulations=../simulations.json --results=../results
