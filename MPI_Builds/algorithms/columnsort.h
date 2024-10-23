#ifndef COLUMN_SORT_H
#define COLUMN_SORT_H

#include <mpi.h>
#include <vector>
#include <string>

// Function to print the status of the matrix
void print_matrix(const std::vector<unsigned int> &vec, int r, int numtasks, const std::string &step, int taskid, MPI_Comm comm);

// Function to sort columns in parallel
void parallel_sort_columns(std::vector<unsigned int> &vec, int taskid, int numtasks, MPI_Comm comm);

// Function to transpose and reshape matrix in parallel
void parallel_transpose_and_reshape(std::vector<unsigned int> &vec, int r, int s, int taskid, int numtasks, MPI_Comm comm);

// Function to untranspose and reshape matrix in parallel
void parallel_untranspose_and_reshape(std::vector<unsigned int> &vec, int r, int s, int taskid, int numtasks, MPI_Comm comm);

// Function to shift columns and manage expansion
void shift(std::vector<unsigned int> &main_vector, int r, int s, int taskid, int numtasks, MPI_Comm comm);

// Main column sort algorithm
int column_sort(std::vector<unsigned int> &main_vector, unsigned int input_size, MPI_Comm worker_comm);

#endif // COLUMN_SORT_H
