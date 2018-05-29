#!/bin/bash

#PBS -l walltime=00:01:00,nodes=7:ppn=4
#PBS -N task1b
#PBS -q batch

for ((i=1; i<=12; ++i))
do
	mpirun --hostfile $PBS_NODEFILE -np $i ./  
done


