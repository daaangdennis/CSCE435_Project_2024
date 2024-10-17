#include <mpi.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
#include <iostream>
#include <caliper/cali.h>

// Print the matrix across all workers
// void printMatrix(const std::vector<unsigned int> &local_matrix, unsigned int local_r, unsigned int local_s, int rank, int size, MPI_Comm comm)
// {
//     std::vector<unsigned int> global_matrix;
//     if (rank == 0)
//     {
//         global_matrix.resize(local_r * local_s * size);
//     }

//     MPI_Gather(local_matrix.data(), local_r * local_s, MPI_UNSIGNED,
//                global_matrix.data(), local_r * local_s, MPI_UNSIGNED, 0, comm);

//     if (rank == 0)
//     {
//         for (unsigned int i = 0; i < local_r; ++i)
//         {
//             for (int p = 0; p < size; ++p)
//             {
//                 for (unsigned int j = 0; j < local_s; ++j)
//                 {
//                     //printf("%u ", global_matrix[p * local_r * local_s + i * local_s + j]);
//                 }
//             }
//             //printf("\n");
//         }
//         //printf("\n");
//     }
// }

// Sort each column of the local matrix
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

// Transpose matrix across workers
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

// Reverse the transpose operation
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

// Shift the matrix to create new arrangement
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

// Undo the shift operation
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

int column_sort(std::vector<unsigned int> &main_vector, unsigned int input_size, MPI_Comm worker_comm)
{
    // Caliper: mark the entire function
    CALI_CXX_MARK_FUNCTION;

    int rank, size;
    MPI_Comm_rank(worker_comm, &rank);
    MPI_Comm_size(worker_comm, &size);

    unsigned int total_elements = input_size * input_size;
    unsigned int r = input_size / size;
    unsigned int s = input_size;

    unsigned int local_r = r;
    unsigned int local_s = s / size;

    if (rank == 0)
    {
        // printf("r (rows per worker) = %u\n", r);
        // printf("s (total number of columns) = %u\n", s);
        // printf("input_size = %u\n", input_size);
        // printf("\n\n");
    }

    std::vector<unsigned int> local_matrix(local_r * local_s);
    MPI_Scatter(main_vector.data(), local_r * local_s, MPI_UNSIGNED,
                local_matrix.data(), local_r * local_s, MPI_UNSIGNED, 0, worker_comm);

    for (int current_rank = 0; current_rank < size; current_rank++)
    {
        if (rank == current_rank)
        {
            // printf("Rank %d Initial Matrix:\n", rank);
            for (unsigned int i = 0; i < local_r; i++)
            {
                for (unsigned int j = 0; j < local_s; j++)
                {
                    // printf("%u ", local_matrix[i * local_s + j]);
                }
                // printf("\n");
            }
            // printf("\n");
        }
        MPI_Barrier(worker_comm);
    }

    // Step 1: Sort columns locally
    CALI_CXX_MARK_LOOP_BEGIN(step1, "Step 1: Sort columns");
    sortColumns(local_matrix, local_r, local_s);
    CALI_CXX_MARK_LOOP_END(step1);
    // if (rank == 0)
    //     // printf("Step 1: Sort columns\n");
    //     printMatrix(local_matrix, local_r, local_s, rank, size, worker_comm);

    // Step 2: Transpose the matrix globally
    CALI_CXX_MARK_LOOP_BEGIN(step2, "Step 2: Transpose");
    transpose(local_matrix, local_r, local_s, rank, size, worker_comm);
    CALI_CXX_MARK_LOOP_END(step2);
    // if (rank == 0)
    //     // printf("Step 2: Transpose\n");
    //     printMatrix(local_matrix, local_r, local_s, rank, size, worker_comm);

    // Step 3: Sort columns again
    CALI_CXX_MARK_LOOP_BEGIN(step3, "Step 3: Sort columns");
    sortColumns(local_matrix, local_r, local_s);
    CALI_CXX_MARK_LOOP_END(step3);
    // if (rank == 0)
    //     // printf("Step 3: Sort columns\n");
    //     printMatrix(local_matrix, local_r, local_s, rank, size, worker_comm);

    // Step 4: Untranspose the matrix
    CALI_CXX_MARK_LOOP_BEGIN(step4, "Step 4: Untranspose");
    untranspose(local_matrix, local_r, local_s, rank, size, worker_comm);
    CALI_CXX_MARK_LOOP_END(step4);
    // if (rank == 0)
    //     // printf("Step 4: Untranspose\n");
    //     printMatrix(local_matrix, local_r, local_s, rank, size, worker_comm);

    // Step 5: Sort columns again
    CALI_CXX_MARK_LOOP_BEGIN(step5, "Step 5: Sort columns");
    sortColumns(local_matrix, local_r, local_s);
    CALI_CXX_MARK_LOOP_END(step5);
    // if (rank == 0)
    //     // printf("Step 5: Sort columns\n");
    //     printMatrix(local_matrix, local_r, local_s, rank, size, worker_comm);

    // Step 6: Shift the matrix
    CALI_CXX_MARK_LOOP_BEGIN(step6, "Step 6: Shift");
    shift(local_matrix, local_r, local_s, rank, size, worker_comm);
    CALI_CXX_MARK_LOOP_END(step6);
    // if (rank == 0)
    //     // printf("Step 6: Shift\n");
    //     printMatrix(local_matrix, local_r, local_s, rank, size, worker_comm);

    // Step 7: Sort columns again
    CALI_CXX_MARK_LOOP_BEGIN(step7, "Step 7: Sort columns");
    sortColumns(local_matrix, local_r, local_s);
    CALI_CXX_MARK_LOOP_END(step7);
    // if (rank == 0)
    //     // printf("Step 7: Sort columns\n");
    //     printMatrix(local_matrix, local_r, local_s, rank, size, worker_comm);

    // Step 8: Unshift the matrix
    CALI_CXX_MARK_LOOP_BEGIN(step8, "Step 8: Unshift");
    unshift(local_matrix, local_r, local_s, rank, size, worker_comm);
    CALI_CXX_MARK_LOOP_END(step8);
    // if (rank == 0)
    //     // printf("Step 8: Unshift (Final result)\n");
    //     printMatrix(local_matrix, local_r, local_s, rank, size, worker_comm);

    MPI_Gather(local_matrix.data(), local_r * local_s, MPI_UNSIGNED,
               main_vector.data(), local_r * local_s, MPI_UNSIGNED, 0, worker_comm);

    return 0;
}