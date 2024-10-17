#ifndef COLUMN_SORT_H
#define COLUMN_SORT_H

#include <mpi.h>
#include <vector>

// void printMatrix(const std::vector<unsigned int> &local_matrix, unsigned int local_r, unsigned int local_s, int rank, int size, MPI_Comm comm);
void sortColumns(std::vector<unsigned int> &local_matrix, unsigned int local_r, unsigned int local_s);
void transpose(std::vector<unsigned int> &local_matrix, unsigned int local_r, unsigned int local_s, int rank, int size, MPI_Comm comm);
void untranspose(std::vector<unsigned int> &local_matrix, unsigned int local_r, unsigned int local_s, int rank, int size, MPI_Comm comm);
void shift(std::vector<unsigned int> &local_matrix, unsigned int &local_r, unsigned int &local_s, int rank, int size, MPI_Comm comm);
void unshift(std::vector<unsigned int> &local_matrix, unsigned int &local_r, unsigned int &local_s, int rank, int size, MPI_Comm comm);
int column_sort(std::vector<unsigned int> &main_vector, unsigned int input_size, MPI_Comm worker_comm);

#endif // COLUMN_SORT_H
