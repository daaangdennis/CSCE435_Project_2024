#ifndef SAMPLESORT_H
#define SAMPLESORT_H

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>


int sample_sort(std::vector<unsigned int> main_vector, unsigned int input_size, MPI_Comm worker_comm);

#endif