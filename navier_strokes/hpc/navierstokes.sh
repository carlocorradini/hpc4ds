#!/bin/bash

#PBS -l nodes=5:ppn=1
#PBS -q short_cpuQ

readonly NUMBER_PROCESSES=5
readonly __DIRNAME=${PWD}

module load mpich-3.2
mpirun.actual -np "${NUMBER_PROCESSES}" "${__DIRNAME}/navierstokes" --simulations="${__DIRNAME}/simulations.json" --results="${__DIRNAME}/results"
