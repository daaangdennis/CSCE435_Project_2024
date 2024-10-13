# Project_2024

### setup
```
./build.sh
make
```

### run grace main
```
sbatch mpi.grace_job <array size> <array type> <algorithm> <processes>
sbatch mpi.grace_job 10 random radix 2
```

### run grace tests
```
sbatch tests/generation-mpi.grace_job 20 2
sbatch tests/validation-mpi.grace_job 20 2
```

This repository contains the necessary materials for the project, including a template for the report
