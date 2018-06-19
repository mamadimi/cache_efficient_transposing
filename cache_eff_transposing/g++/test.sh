#!/bin/bash

percentage=100;
parts=0;

#bins mod 2 =0
bins=256

for n in 20 21 22
do
     N=$((2 ** n));
    ./generate_perm_vector $N $parts $percentage $memory

    ./simple_transposing $N >> serialReadFromInput$N.txt

    ./cache_eff_transposing $N $bins >> improvedSerialReadFromInput$N.txt
done

rm -rf permutationVector.txt
