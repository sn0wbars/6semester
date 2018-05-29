#!/bin/bash
max_p=20
file=res3.txt
rm $file

printf "[" >> $file
for ((i=1; i<=max_p; ++i))
do
   ./parprogT3 $i
    if ((i<max_p))
    then
    	printf ", " >> $file
    fi
done
printf "]" >> $file 
