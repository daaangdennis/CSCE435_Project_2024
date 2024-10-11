#ifndef DECENTRALIZED_GENERATION_H
#define DECENTRALIZED_GENERATION_H

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdlib.h>
#include <random>
#include <time.h>
#include <vector>

#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

/*
    - input
        - pid: processor id
        - num_processors: number of processors running in parallel
        - global_seq_size: the size of the global sequence
        - flag
            - 0: sorted
            - 1: reverse sorted
            - 2: 1% pertubed (1% of the global_seq_size)
            - 3: random
    - output: a local sequence of random numbers
    - assumptions
        - numbers can repeat
        - no guarantee every processor's local sequence will be the same size
*/
std::vector<unsigned int> decentralized_generation(
    const int& pid, const int& num_processors, const unsigned int& global_seq_size, const unsigned int& flag
);

#endif
