#!/bin/bash

max_p=20
file=res4.txt
rm $file

printf "[" >> $file
for ((i=1; i<=max_p; ++i))
do
   ./parprogT4 $i $1
    if ((i<max_p))
    then
    	printf ", " >> $file
    fi
done
printf "]" >> $file 
