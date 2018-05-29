#!/bin/bash

#PBS -l walltime=00:05:00,nodes=7:ppn=4
#PBS -N task1b
#PBS -q batch

cd $PBS_O_WORKDIR

max_p=20
pow=8
file=res1.txt

rm $file

printf "[" >> $file
for ((k=0; k<=pow; ++k))
do
    printf "[" >> $file
    for ((i=1; i<=$max_p; ++i))
    do
        mpirun --hostfile $PBS_NODEFILE -np $i ./paralT1 $((10**k)) >> $file
        if ((i<max_p))
        then
        	printf "," >> $file
        fi
    done
    printf "]" >> $file
    if ((k<pow))
    then
    	printf "," >> $file
    fi
done
printf "]" >> $file 
