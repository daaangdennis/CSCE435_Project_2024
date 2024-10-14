#ifndef BITONICSORT_H
#define BITONICSORT_H

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

#include <algorithm>
#include <vector>


void bitonic_sort(std::vector<unsigned int>& local_seq, const int& taskid, const int& numtasks, const MPI_Comm& comm);

#endif