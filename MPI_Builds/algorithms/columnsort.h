#ifndef COLUMNSORT_H
#define COLUMNSORT_H

#include <mpi.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
#include <iostream>
#include <iomanip>
#include <caliper/cali.h>

void print_matrix(const std::vector<unsigned int> &local_vector, unsigned int n, unsigned int r, unsigned int s,
                  const std::string &step, int taskid, MPI_Comm comm);

void parallel_transpose_and_reshape(std::vector<unsigned int> &local_vector, unsigned int r, unsigned int s,
                                    int taskid, MPI_Comm comm);

void parallel_untranspose_and_reshape(std::vector<unsigned int> &local_vector, unsigned int r, unsigned int s,
                                      int taskid, MPI_Comm comm);

void parallel_shift_and_unshift(std::vector<unsigned int> &local_vector, unsigned int n, unsigned int r, unsigned int s,
                                int taskid, MPI_Comm comm);

void column_sort(std::vector<unsigned int> &local_vector, unsigned int n, unsigned int r, unsigned int s,
                 int taskid, MPI_Comm comm);

#endif // COLUMNSORT_H