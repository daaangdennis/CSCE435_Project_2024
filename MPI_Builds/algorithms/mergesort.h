#ifndef MERGESORT_H
#define MERGESORT_H

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <vector>
#include <algorithm>
#include <numeric>

#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>


int mergesort(std::vector<unsigned int> main_vector, unsigned int input_size, const int& numtasks, MPI_Comm worker_comm);

#endif