#ifndef SAMPLESORT_H
#define SAMPLESORT_H

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

#include <algorithm>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

/*
    - input
        - local_seq: locally stored sequence
        - pid: processor id
        - num_processors: number of processors running in parallel
        - comm: communicator
        - k: oversampling factor (how many number to sample from the local sequence)
    - output: 
        - modifies the local sequence on each processor
        - ensuring concatenated local sequences result in a sorted global sequence
    - assumptions:
        - more than 1 processor running in parallel
        - 64-bit system (i.e. sizeof(size_t) == sizeof(unsigned long long))
        - 0 < K <= local_seq.size()
*/
void samplesort(
    std::vector<unsigned int>& local_seq, 
    const int& pid, const int& num_processors, const MPI_Comm& comm,
    const unsigned long long& K
);

#endif
