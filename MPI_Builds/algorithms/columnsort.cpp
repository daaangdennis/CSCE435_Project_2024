#include <mpi.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
#include <iostream>
#include <caliper/cali.h>
#include "columnsort.h"

// Function to print the status of the matrix
void print_matrix(const std::vector<unsigned int> &vec, int r, int numtasks, const std::string &step, int taskid, MPI_Comm comm)
{
    std::vector<unsigned int> full_vector;
    if (taskid == 0)
    {
        full_vector.resize(r); // Resize to hold all elements
    }

    // Gather data from all tasks
    MPI_Gather(vec.data(), r / numtasks, MPI_UNSIGNED, full_vector.data(), r / numtasks, MPI_UNSIGNED, 0, comm);

    if (taskid == 0)
    {
        std::cout << step << ":\n";

        // Print task IDs as column headers
        for (int t = 0; t < numtasks; ++t)
        {
            std::cout << "Task " << t << "\t";
        }
        std::cout << std::endl;

        // Print each task's results in a formatted way
        for (int i = 0; i < r / numtasks; ++i)
        {
            for (int t = 0; t < numtasks; ++t)
            {
                std::cout << full_vector[t * (r / numtasks) + i] << '\t'; // Use tab for better formatting
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}

void parallel_sort_columns(std::vector<unsigned int> &vec, int taskid, int numtasks, MPI_Comm comm)
{
    std::sort(vec.begin(), vec.end());
}

void parallel_transpose_and_reshape(std::vector<unsigned int> &vec, int r, int s, int taskid, int numtasks, MPI_Comm comm)
{
    int local_r = r / numtasks;
    std::vector<unsigned int> reshaped_vector(local_r); // Vector to hold reshaped data

    // Buffer to send and receive data for the transposition
    std::vector<unsigned int> transposed_vector(local_r);
    std::vector<unsigned int> recv_buffer(local_r);

    // Step 1: Transpose - exchanging elements between processes to get the right columns
    for (int i = 0; i < local_r; ++i)
    {
        // Compute source and destination positions
        int global_index = taskid * local_r + i; // Global index in the original matrix
        int source_col = global_index % numtasks;
        int source_row = global_index / numtasks;

        // Send elements to appropriate processes
        int dest_task = source_col;
        int send_offset = source_row;

        MPI_Sendrecv(&vec[i], 1, MPI_UNSIGNED, dest_task, 0,
                     &recv_buffer[i], 1, MPI_UNSIGNED, dest_task, 0,
                     comm, MPI_STATUS_IGNORE);
    }

    // Copy received elements into the reshaped vector
    for (int i = 0; i < local_r; ++i)
    {
        transposed_vector[i] = recv_buffer[i];
    }

    // Step 2: Reshaping - reassign positions locally
    for (int i = 0; i < local_r; ++i)
    {
        int new_row = i / numtasks;
        int new_col = i % numtasks;
        reshaped_vector[new_row * numtasks + new_col] = transposed_vector[i];
    }

    // Update the original vector with the reshaped data
    vec = reshaped_vector;
}

void parallel_untranspose_and_reshape(std::vector<unsigned int> &vec, int r, int s, int taskid, int numtasks, MPI_Comm comm)
{
    int local_r = r / numtasks;
    std::vector<unsigned int> untransposed_vector(local_r);
    std::vector<unsigned int> recv_buffer(local_r);
    std::vector<unsigned int> reshaped_vector(local_r);

    // Step 1: Untranspose - exchanging elements between processes
    for (int i = 0; i < local_r; ++i)
    {
        int global_index = taskid * local_r + i;
        int dest_col = global_index % numtasks;
        int dest_row = global_index / numtasks;

        int dest_task = dest_row % numtasks;
        int send_offset = dest_col;

        MPI_Sendrecv(&vec[i], 1, MPI_UNSIGNED, dest_task, 0,
                     &recv_buffer[i], 1, MPI_UNSIGNED, dest_task, 0,
                     comm, MPI_STATUS_IGNORE);
    }

    // Copy received elements into the untransposed vector
    for (int i = 0; i < local_r; ++i)
    {
        untransposed_vector[i] = recv_buffer[i];
    }

    // Step 2: Reshaping - reassign positions locally
    for (int i = 0; i < local_r; ++i)
    {
        int new_row = i / s;
        int new_col = i % s;
        reshaped_vector[new_row * s + new_col] = untransposed_vector[i];
    }

    // Update the original vector with the reshaped data
    vec = reshaped_vector;
}

void shift(std::vector<unsigned int> &main_vector, int r, int s, int taskid, int numtasks, MPI_Comm comm)
{
    int column_size = main_vector.size();
    int local_r = r / numtasks;
    int expanded_size = column_size + r;
    std::vector<unsigned int> expanded_vector(column_size + r, UINT_MAX); // Using UINT_MAX as infinity

    // Step 1: Copy original values to expanded vector with offset of local_r
    for (int i = 0; i < local_r; i++)
    {
        expanded_vector[(column_size / numtasks) + i] = main_vector[i];
    }

    // Print expanded matrix
    print_matrix(expanded_vector, r + column_size, numtasks, "Step 1: Expanded matrix", taskid, comm);

    // Step 2: Handle processor 0's shift behavior
    if (taskid == 0)
    {
        std::vector<unsigned int> temp_vector;
        int shift_amount = local_r / 2;

        // Collect elements to be shifted
        for (int i = 0; i < shift_amount; i++)
        {
            if (expanded_vector[i] != UINT_MAX)
            {
                temp_vector.push_back(expanded_vector[(expanded_size / numtasks) - 1]);
                std::cout << "Pushed value: " << (expanded_size / numtasks) - 1 << expanded_vector[(expanded_size / numtasks) - 1] << std::endl;
                expanded_vector.pop_back();
                expanded_vector.insert(expanded_vector.begin(), UINT_MAX);
            }
        }

        // Send these elements to the next processor
        if (numtasks > 1)
        {
            MPI_Send(temp_vector.data(), temp_vector.size(), MPI_UNSIGNED, 1, 0, comm);
        }

        for (int i = 0; i < shift_amount; i++)
        {
            expanded_vector[i] = 0;
        }
    }
    print_matrix(expanded_vector, r + column_size, numtasks, "Step 2: Starting processor", taskid, comm);

    // Step 3 : Middle processors behavior else if (taskid < numtasks - 1)
    {
        // Receive shifted elements from previous processor
        MPI_Status status;
        MPI_Probe(taskid - 1, 0, comm, &status);
        int recv_count;
        MPI_Get_count(&status, MPI_UNSIGNED, &recv_count);

        std::vector<unsigned int> received_vector(recv_count);
        MPI_Recv(received_vector.data(), recv_count, MPI_UNSIGNED, taskid - 1, 0, comm, MPI_STATUS_IGNORE);

        // Insert received elements at the beginning
        for (int i = 0; i < recv_count; i++)
        {
            expanded_vector[i] = received_vector[i];
        }

        print_matrix(expanded_vector, r + column_size, numtasks, "After recieving from previous processor", taskid, comm);

        // // Sort the expanded vector
        // std::sort(expanded_vector.begin(), expanded_vector.end());

        // // Collect elements to be shifted to next processor
        // std::vector<unsigned int> elements_to_shift;
        // for (int i = expanded_size - recv_count; i < expanded_size; i++)
        // {
        //     if (expanded_vector[i] == UINT_MAX)
        //     {
        //         elements_to_shift.push_back(expanded_vector[i]);
        //     }
        // }

        // // Send elements to next processor and receive elements from next processor
        // int send_count = elements_to_shift.size();
        // std::vector<unsigned int> received_from_next(send_count);

        // // Use MPI_Sendrecv to handle both operations atomically
        // MPI_Sendrecv(elements_to_shift.data(), send_count, MPI_UNSIGNED, taskid + 1, 0,
        //              received_from_next.data(), send_count, MPI_UNSIGNED, taskid + 1, 0,
        //              comm, MPI_STATUS_IGNORE);

        // // Replace the INF elements at the end with received elements
        // int replace_start = expanded_size - send_count;
        // for (int i = 0; i < send_count; i++)
        // {
        //     expanded_vector[replace_start + i] = received_from_next[i];
        // }
    }
    print_matrix(expanded_vector, r + column_size, numtasks, "Step 3: middle processor", taskid, comm);

    // // Step 4: Last processor behavior
    // else
    // {
    //     // Receive shifted elements from previous processor
    //     MPI_Status status;
    //     MPI_Probe(taskid - 1, 0, comm, &status);
    //     int recv_count;
    //     MPI_Get_count(&status, MPI_UNSIGNED, &recv_count);

    //     std::vector<unsigned int> received_vector(recv_count);
    //     MPI_Recv(received_vector.data(), recv_count, MPI_UNSIGNED, taskid - 1, 0, comm, MPI_STATUS_IGNORE);

    //     // Insert received elements at the beginning
    //     for (int i = 0; i < recv_count; i++)
    //     {
    //         expanded_vector[i] = received_vector[i];
    //     }

    //     // Sort the expanded vector
    //     std::sort(expanded_vector.begin(), expanded_vector.end());
    // }
    // print_matrix(expanded_vector, r + column_size, numtasks, "Step 4: final processor", taskid, comm);

    // // Synchronize all processes
    // MPI_Barrier(comm);
}

int column_sort(std::vector<unsigned int> &main_vector, unsigned int input_size, MPI_Comm worker_comm)
{
    // Caliper: mark the entire function
    CALI_CXX_MARK_FUNCTION;

    int rank, size;
    MPI_Comm_rank(worker_comm, &rank);
    MPI_Comm_size(worker_comm, &size);

    int r = array_size;
    int s = numtasks;
    int local_r = r / numtasks;

    print_matrix(main_vector, r, numtasks, "Begin result before column sort", taskid, MPI_COMM_WORLD);

    // Step 1: Sort columns
    std::sort(main_vector.begin(), main_vector.end());
    print_matrix(main_vector, r, numtasks, "Step 1: Sort columns", taskid, MPI_COMM_WORLD);

    // Step 2: Transpose and Reshape
    parallel_transpose_and_reshape(main_vector, r, s, taskid, numtasks, MPI_COMM_WORLD);
    print_matrix(main_vector, r, numtasks, "Step 2: Transpose and Reshape", taskid, MPI_COMM_WORLD);

    // Step 3: Sort columns
    std::sort(main_vector.begin(), main_vector.end());
    print_matrix(main_vector, r, numtasks, "Step 3: Sort columns", taskid, MPI_COMM_WORLD);

    // Step 4: Untranspose and Reshape
    parallel_untranspose_and_reshape(main_vector, r, s, taskid, numtasks, MPI_COMM_WORLD);
    print_matrix(main_vector, r, numtasks, "Step 4: Untranspose and Reshape", taskid, MPI_COMM_WORLD);

    // Step 5: Sort columns
    std::sort(main_vector.begin(), main_vector.end());
    print_matrix(main_vector, r, numtasks, "Step 5: Sort columns", taskid, MPI_COMM_WORLD);

    // Step 6: expand the columns
    shift(main_vector, r, s, taskid, numtasks, MPI_COMM_WORLD);
    // print_matrix(main_vector, r, numtasks, "Step 6: Expand", taskid, MPI_COMM_WORLD);
}