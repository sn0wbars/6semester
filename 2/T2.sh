#!/bin/bash

#PBS -l walltime=00:05:00,nodes=6:ppn=4
#PBS -N task1b
#PBS -q batch

cd $PBS_O_WORKDIR

max_p=20
file=res2.txt
a=(2000 5000 10000 20000 30000 40000 50000)
r=${#a[*]}

rm $file

printf "[" >> $file
for ((k=0; k<r; ++k))
do
    printf "[" >> $file
    for ((i=1; i<=max_p; ++i))
    do
        mpirun --hostfile $PBS_NODEFILE -np $i ./paralT2 ${a[$k]}
        if ((i<max_p))
        then
        	printf ", " >> $file
        fi
    done
    printf "]" >> $file
    if ((k<r-1))
    then
    	printf ", " >> $file
    fi
done
printf "]" >> $file 