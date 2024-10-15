# Project_2024

### setup
```
./build.sh
make
```

### array type flags
```
0: sorted
1: reverse
2: 1% perturbed 
3: random
```

### run radix sort
```
sbatch radix.grace_job 2^<array size> <array type> <processes>
sbatch radix.grace_job 10 3 2
```

### run grace tests
```
sbatch tests/generation-mpi.grace_job 20 2
sbatch tests/validation-mpi.grace_job 20 2
```

This repository contains the necessary materials for the project, including a template for the report
