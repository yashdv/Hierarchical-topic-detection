#!/bin/bash

counter=0

for i in `ls`
do
    if [ "$i" = "run.sh" ]
    then
        continue;
    fi

    counter=$((counter + 1))
    mv $i $counter
done
