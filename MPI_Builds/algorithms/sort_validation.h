#ifndef SORT_VALIDATION_H
#define SORT_VALIDATION_H

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <limits.h>
#include <algorithm>
#include <vector>

#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

/*
    - input
        - local_seq: locally stored sequence
        - pid: processor id
        - num_processors: number of processors running in parallel
        - comm: communicator
    - output: true if the global sequence is sorted, false otherwise
    - assumptions:
        - more than 1 processor running in parallel
        - 64-bit system (i.e. sizeof(size_t) == sizeof(unsigned long long))
*/
bool sort_validation(const std::vector<unsigned int>& local_seq, const int& pid, const int& num_processors, const MPI_Comm& comm);

#endif
