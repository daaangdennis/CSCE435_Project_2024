# Project_2024

### setup

```
./build.sh
make
```

### run main

```
sbatch mpi.grace_job <processor> <array size> <array type> <algorithm>

note: array size is the power of the input size

ex:
sbatch mpi.grace_job 2 16 random radix

processors = 2 | array size = 2^16 | array type = random | algorithm = radix
```

### options for array type:

```
sorted
random
perturbed
reverse
```

### options for algorithms:

```
bitonic
column
merge
radix
sample
```

<!-- ### setup

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

This repository contains the necessary materials for the project, including a template for the report -->
