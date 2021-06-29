#!/bin/bash

#PBS -l nodes=8:ppn=1
#PBS -q short_cpuQ

readonly NUMBER_PROCESSES=8

module load mpich-3.2
mpirun.actual -np "${NUMBER_PROCESSES}" ./navierstokes --simulations=./simulation_wall.json --results=./results