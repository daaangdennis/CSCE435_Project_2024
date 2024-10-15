#!/bin/bash

# Values for a, b, and c
arr_size=(16 18 20 22 24 26 28)
arr_type_flag=(3)
procs=(512)

# Loop over each value of c
for c in "${procs[@]}"; do
    for a in "${arr_size[@]}"; do
        # For each combination of a, b, and c, run sbatch
        for b in "${arr_type_flag[@]}"; do
            echo "Running: sbatch radix.grace_job $a $b $c"
            sbatch radix.grace_job "$a" "$b" "$c"
        done
    done
done

