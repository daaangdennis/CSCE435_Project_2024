#include <mpi.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
#include <iostream>

void printMatrix(const std::vector<unsigned int> &local_matrix, unsigned int local_r, unsigned int local_s, int rank, int size, MPI_Comm comm)
{
    std::vector<unsigned int> global_matrix;
    if (rank == 0)
    {
        global_matrix.resize(local_r * local_s * size);
    }

    MPI_Gather(local_matrix.data(), local_r * local_s, MPI_UNSIGNED,
               global_matrix.data(), local_r * local_s, MPI_UNSIGNED, 0, comm);

    if (rank == 0)
    {
        for (unsigned int i = 0; i < local_r; ++i)
        {
            for (int p = 0; p < size; ++p)
            {
                for (unsigned int j = 0; j < local_s; ++j)
                {
                    std::cout << global_matrix[p * local_r * local_s + i * local_s + j] << " ";
                }
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
}

void sortColumns(std::vector<unsigned int> &local_matrix, unsigned int local_r, unsigned int local_s)
{
    for (unsigned int j = 0; j < local_s; ++j)
    {
        std::vector<unsigned int> column;
        for (unsigned int i = 0; i < local_r; ++i)
        {
            column.push_back(local_matrix[i * local_s + j]);
        }
        std::sort(column.begin(), column.end());
        for (unsigned int i = 0; i < local_r; ++i)
        {
            local_matrix[i * local_s + j] = column[i];
        }
    }
}

void transpose(std::vector<unsigned int> &local_matrix, unsigned int local_r, unsigned int local_s, int rank, int size, MPI_Comm comm)
{
    std::vector<unsigned int> temp = local_matrix;
    std::vector<unsigned int> global_matrix(local_r * local_s * size);

    MPI_Allgather(temp.data(), local_r * local_s, MPI_UNSIGNED,
                  global_matrix.data(), local_r * local_s, MPI_UNSIGNED, comm);

    for (unsigned int i = 0; i < local_r; ++i)
    {
        for (unsigned int j = 0; j < local_s * size; ++j)
        {
            unsigned int global_idx = j * local_r + i;
            unsigned int proc = global_idx / (local_r * local_s);
            unsigned int local_idx = global_idx % (local_r * local_s);
            if (proc == rank)
            {
                local_matrix[local_idx] = global_matrix[i * local_s * size + j];
            }
        }
    }
}

void untranspose(std::vector<unsigned int> &local_matrix, unsigned int local_r, unsigned int local_s, int rank, int size, MPI_Comm comm)
{
    std::vector<unsigned int> temp = local_matrix;
    std::vector<unsigned int> global_matrix(local_r * local_s * size);

    MPI_Allgather(temp.data(), local_r * local_s, MPI_UNSIGNED,
                  global_matrix.data(), local_r * local_s, MPI_UNSIGNED, comm);

    for (unsigned int i = 0; i < local_r * size; ++i)
    {
        for (unsigned int j = 0; j < local_s; ++j)
        {
            unsigned int global_idx = i * local_s + j;
            unsigned int proc = global_idx / (local_r * local_s);
            unsigned int local_idx = global_idx % (local_r * local_s);
            if (proc == rank)
            {
                local_matrix[local_idx] = global_matrix[j * local_r * size + i];
            }
        }
    }
}

void shift(std::vector<unsigned int> &local_matrix, unsigned int &local_r, unsigned int &local_s, int rank, int size, MPI_Comm comm)
{
    unsigned int shift_amount = local_r * size / 2;
    std::vector<unsigned int> global_matrix(local_r * local_s * size);
    std::vector<unsigned int> shifted_global_matrix((local_r * size) * (local_s * size + 1), std::numeric_limits<unsigned int>::max());

    MPI_Allgather(local_matrix.data(), local_r * local_s, MPI_UNSIGNED,
                  global_matrix.data(), local_r * local_s, MPI_UNSIGNED, comm);

    if (rank == 0)
    {
        for (unsigned int i = 0; i < local_r * size; ++i)
        {
            for (unsigned int j = 0; j < local_s * size; ++j)
            {
                unsigned int new_row = i + shift_amount;
                if (new_row < local_r * size)
                {
                    shifted_global_matrix[new_row * (local_s * size + 1) + j] = global_matrix[i * local_s * size + j];
                }
                else
                {
                    shifted_global_matrix[(new_row - local_r * size) * (local_s * size + 1) + j + 1] = global_matrix[i * local_s * size + j];
                }
            }
        }

        for (unsigned int i = 0; i < shift_amount; ++i)
        {
            shifted_global_matrix[i * (local_s * size + 1)] = 0;
        }
    }

    MPI_Bcast(shifted_global_matrix.data(), shifted_global_matrix.size(), MPI_UNSIGNED, 0, comm);

    local_s++;
    local_matrix.resize(local_r * local_s);

    for (unsigned int i = 0; i < local_r; ++i)
    {
        for (unsigned int j = 0; j < local_s; ++j)
        {
            local_matrix[i * local_s + j] = shifted_global_matrix[(i * size + rank) * (local_s * size) + j];
        }
    }
}

void unshift(std::vector<unsigned int> &local_matrix, unsigned int &local_r, unsigned int &local_s, int rank, int size, MPI_Comm comm)
{
    unsigned int shift_amount = local_r * size / 2;
    std::vector<unsigned int> global_matrix(local_r * local_s * size);
    std::vector<unsigned int> unshifted_global_matrix(local_r * size * (local_s * size - 1));

    MPI_Allgather(local_matrix.data(), local_r * local_s, MPI_UNSIGNED,
                  global_matrix.data(), local_r * local_s, MPI_UNSIGNED, comm);

    if (rank == 0)
    {
        for (unsigned int i = 0; i < local_r * size; ++i)
        {
            for (unsigned int j = 0; j < local_s * size - 1; ++j)
            {
                unsigned int old_row = (i + shift_amount) % (local_r * size);
                unsigned int old_col = (old_row < shift_amount) ? j + 1 : j;
                unshifted_global_matrix[i * (local_s * size - 1) + j] = global_matrix[old_row * local_s * size + old_col];
            }
        }
    }

    MPI_Bcast(unshifted_global_matrix.data(), unshifted_global_matrix.size(), MPI_UNSIGNED, 0, comm);

    local_s--;
    local_matrix.resize(local_r * local_s);

    for (unsigned int i = 0; i < local_r; ++i)
    {
        for (unsigned int j = 0; j < local_s; ++j)
        {
            local_matrix[i * local_s + j] = unshifted_global_matrix[(i * size + rank) * (local_s * size) + j];
        }
    }
}

int column_sort(std::vector<unsigned int> main_vector, unsigned int input_size, MPI_Comm worker_comm)
{
    int rank, size;
    MPI_Comm_rank(worker_comm, &rank);
    MPI_Comm_size(worker_comm, &size);

    unsigned int r = std::sqrt(input_size);
    unsigned int s = r;
    unsigned int local_s = s / size;
    unsigned int local_r = r;

    std::vector<unsigned int> local_matrix(local_r * local_s);
    MPI_Scatter(main_vector.data(), local_r * local_s, MPI_UNSIGNED,
                local_matrix.data(), local_r * local_s, MPI_UNSIGNED, 0, worker_comm);

    if (rank == 0)
        std::cout << "Start:\n";
    printMatrix(local_matrix, local_r, local_s, rank, size, worker_comm);

    sortColumns(local_matrix, local_r, local_s);
    if (rank == 0)
        std::cout << "Step 1: Sort columns\n";
    printMatrix(local_matrix, local_r, local_s, rank, size, worker_comm);

    transpose(local_matrix, local_r, local_s, rank, size, worker_comm);
    if (rank == 0)
        std::cout << "Step 2: Transpose\n";
    printMatrix(local_matrix, local_r, local_s, rank, size, worker_comm);

    sortColumns(local_matrix, local_r, local_s);
    if (rank == 0)
        std::cout << "Step 3: Sort columns\n";
    printMatrix(local_matrix, local_r, local_s, rank, size, worker_comm);

    untranspose(local_matrix, local_r, local_s, rank, size, worker_comm);
    if (rank == 0)
        std::cout << "Step 4: Untranspose\n";
    printMatrix(local_matrix, local_r, local_s, rank, size, worker_comm);

    sortColumns(local_matrix, local_r, local_s);
    if (rank == 0)
        std::cout << "Step 5: Sort columns\n";
    printMatrix(local_matrix, local_r, local_s, rank, size, worker_comm);

    shift(local_matrix, local_r, local_s, rank, size, worker_comm);
    if (rank == 0)
        std::cout << "Step 6: Shift\n";
    printMatrix(local_matrix, local_r, local_s, rank, size, worker_comm);

    sortColumns(local_matrix, local_r, local_s);
    if (rank == 0)
        std::cout << "Step 7: Sort columns\n";
    printMatrix(local_matrix, local_r, local_s, rank, size, worker_comm);

    unshift(local_matrix, local_r, local_s, rank, size, worker_comm);
    if (rank == 0)
        std::cout << "Step 8: Unshift (Final result)\n";
    printMatrix(local_matrix, local_r, local_s, rank, size, worker_comm);

    MPI_Gather(local_matrix.data(), local_r * local_s, MPI_UNSIGNED,
               main_vector.data(), local_r * local_s, MPI_UNSIGNED, 0, worker_comm);

    return 0;
}