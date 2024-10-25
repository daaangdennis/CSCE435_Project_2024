#include "columnsort.h"

// void print_matrix(const std::vector<unsigned int> &local_vector, unsigned int n, unsigned int r, unsigned int s, const std::string &step, int taskid, MPI_Comm comm)
// {
//     std::vector<unsigned int> full_vector;
//     if (taskid == 0)
//     {
//         full_vector.resize(n); // Resize to hold all elements
//     }

//     // Gather data from all tasks
//     MPI_Gather(local_vector.data(), r, MPI_UNSIGNED, full_vector.data(), r, MPI_UNSIGNED, 0, comm);

//     if (taskid == 0)
//     {
//         std::cout << step << ":\n\n";

//         // Print task IDs as column headers
//         for (unsigned int i = 0; i < s; ++i)
//         {
//             std::cout << "Task " << i << "\t";
//         }
//         std::cout << std::endl;

//         // Print each task's results in a matrix format
//         for (unsigned int i = 0; i < r; ++i)
//         {
//             for (unsigned int j = 0; j < s; ++j)
//             {
//                 std::cout << full_vector[j * (r) + i] << '\t';
//             }
//             std::cout << std::endl;
//         }
//         std::cout << std::endl;
//     }
// }

void parallel_transpose_and_reshape(std::vector<unsigned int> &local_vector, unsigned int r, unsigned int s, int taskid, MPI_Comm comm)
{
    std::vector<unsigned int> reshaped_vector(r);
    std::vector<unsigned int> transposed_vector(r);
    std::vector<unsigned int> recv_buffer(r);

    CALI_MARK_BEGIN("comm");
    // Step 1: Transpose - exchanging elements between processes
    for (unsigned int i = 0; i < r; ++i)
    {
        unsigned int global_index = taskid * r + i;
        unsigned int source_col = global_index % s;
        unsigned int source_row = global_index / s;
        unsigned int dest_task = source_col;
        unsigned int send_offset = source_row;

        MPI_Sendrecv(&local_vector[i], 1, MPI_UNSIGNED, dest_task, 0,
                     &recv_buffer[i], 1, MPI_UNSIGNED, dest_task, 0,
                     comm, MPI_STATUS_IGNORE);
    }
    CALI_MARK_END("comm");

    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    // Copy received elements
    for (unsigned int i = 0; i < r; ++i)
    {
        transposed_vector[i] = recv_buffer[i];
    }

    // Step 2: Reshaping
    for (unsigned int i = 0; i < r; ++i)
    {
        unsigned int new_row = i / s;
        unsigned int new_col = i % s;
        reshaped_vector[new_row * s + new_col] = transposed_vector[i];
    }

    local_vector = reshaped_vector;
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");
}

void parallel_untranspose_and_reshape(std::vector<unsigned int> &local_vector, unsigned int r, unsigned int s, int taskid, MPI_Comm comm)
{
    unsigned int segment_size = r / s;

    std::vector<unsigned int> all_data(r * s);
    std::vector<unsigned int> final_vector(r);

    CALI_MARK_BEGIN("comm");
    // Step 1: Gather all data to all processes
    MPI_Allgather(local_vector.data(), r, MPI_UNSIGNED,
                  all_data.data(), r, MPI_UNSIGNED,
                  comm);
    CALI_MARK_END("comm");

    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    // Step 2: Calculate which elements this task should keep
    unsigned int temp = taskid * segment_size;

    // Step 3: Redistribute the data
    for (unsigned int i = 0; i < segment_size; i++)
    {
        for (unsigned int j = 0; j < s; j++)
        {
            // Calculate source position in all_data
            unsigned int source_idx = j * r + i + temp;
            final_vector[i * s + j] = all_data[source_idx];
        }
    }

    // Update the original vector with untransposed data
    local_vector = final_vector;
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");
}

void parallel_shift_and_unshift(std::vector<unsigned int> &local_vector, unsigned int n, unsigned int r, unsigned int s, int taskid, MPI_Comm comm)
{
    unsigned int shift_size = r / 2;
    std::vector<unsigned int> send_buffer(shift_size);
    std::vector<unsigned int> recv_buffer(shift_size);
    std::vector<unsigned int> send_buffer_unshift(shift_size);
    std::vector<unsigned int> recv_buffer_unshift(shift_size);

    /*----------------- Part 1 SHIFT*/

    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_small");
    // Step 1: Copy elements to send buffer (last r/2 elements) while popping them from the main vector
    for (unsigned int i = 0; i < shift_size; i++)
    {
        send_buffer[i] = local_vector[(r - 1) - i];
        local_vector.pop_back();
    }
    CALI_MARK_END("comp_small");
    CALI_MARK_END("comp");

    CALI_MARK_BEGIN("comm");
    // Use MPI_Sendrecv to simultaneously send and receive
    MPI_Sendrecv(send_buffer.data(), shift_size, MPI_UNSIGNED,
                 (taskid + 1) % s, 0,
                 recv_buffer.data(), shift_size, MPI_UNSIGNED,
                 (taskid - 1 + s) % s, 0,
                 comm, MPI_STATUS_IGNORE);
    CALI_MARK_END("comm");

    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    // Starting processor behavior
    if (taskid == 0)
    {
        // Step 2: Insert 0 shift_size times at the beginning
        for (unsigned int i = 0; i < shift_size; i++)
        {
            local_vector.insert(local_vector.begin(), 0);
        }
    }

    // Rest of the processor behavior
    if (taskid != 0)
    {
        // Step 3: Insert received elements at the beginning of vector
        for (unsigned int i = 0; i < shift_size; i++)
        {
            local_vector.insert(local_vector.begin(), recv_buffer[i]);
        }
    }
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");

    // Step 6
    // print_matrix(local_vector, n, r, s, "Step 6: Shift", taskid, MPI_COMM_WORLD);

    /*----------------- Part 2 SORT*/

    // Step 7
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_small");
    std::sort(local_vector.begin(), local_vector.end());
    CALI_MARK_END("comp_small");
    CALI_MARK_END("comp");
    // print_matrix(local_vector, n, r, s, "Step 7: Sort Columns", taskid, MPI_COMM_WORLD);

    /*----------------- Part 3 UNSHIFT*/

    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_small");
    // Step 1: Copy elements to send buffer unshift (first r/2 elements)
    for (unsigned int i = 0; i < shift_size; i++)
    {
        send_buffer_unshift[i] = local_vector[i];
    }

    // Erase these elements only after copying them
    local_vector.erase(local_vector.begin(), local_vector.begin() + shift_size);
    CALI_MARK_END("comp_small");
    CALI_MARK_END("comp");

    CALI_MARK_BEGIN("comm");
    // Use MPI_Sendrecv to simultaneously send and receive
    MPI_Sendrecv(send_buffer_unshift.data(), shift_size, MPI_UNSIGNED,
                 (taskid - 1 + s) % s, 0,
                 recv_buffer_unshift.data(), shift_size, MPI_UNSIGNED,
                 (taskid + 1) % s, 0,
                 comm, MPI_STATUS_IGNORE);
    CALI_MARK_END("comm");

    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    // Ending processor behavior
    if (taskid == s - 1)
    {
        // Step 2: Insert sending buffer (left over numbers from shift part)
        for (unsigned int i = 0; i < shift_size; i++)
        {
            local_vector.push_back(send_buffer[shift_size - 1 - i]);
        }
    }

    // Rest of the processor behavior
    if (taskid != s - 1)
    {
        // Step 3: Insert received elements at the end of vector
        for (unsigned int i = 0; i < shift_size; i++)
        {
            local_vector.push_back(recv_buffer_unshift[i]);
        }
    }
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");
    // Step 8
    // print_matrix(local_vector, n, r, s, "Step 8: Unshift", taskid, MPI_COMM_WORLD);

    // Synchronize all processors
    CALI_MARK_BEGIN("comm");
    MPI_Barrier(comm);
    CALI_MARK_END("comm");
}

void column_sort(std::vector<unsigned int> &local_vector, unsigned int n, unsigned int r, unsigned int s, int taskid, MPI_Comm comm)
{
    // print_matrix(local_vector, n, r, s, "Beginning matrix:", taskid, comm);

    // Step 1: Sort columns
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_small");
    std::sort(local_vector.begin(), local_vector.end());
    CALI_MARK_END("comp_small");
    CALI_MARK_END("comp");
    // print_matrix(local_vector, n, r, s, "Step 1: Sort columns", taskid, comm);

    // Step 2: Transpose and Reshape
    parallel_transpose_and_reshape(local_vector, r, s, taskid, comm);
    // print_matrix(local_vector, n, r, s, "Step 2: Transpose and Reshape", taskid, comm);

    // Step 3: Sort columns
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_small");
    std::sort(local_vector.begin(), local_vector.end());
    CALI_MARK_END("comp_small");
    CALI_MARK_END("comp");
    // print_matrix(local_vector, n, r, s, "Step 3: Sort columns", taskid, comm);

    // Step 4: Untranspose and Reshape
    parallel_untranspose_and_reshape(local_vector, r, s, taskid, comm);
    // print_matrix(local_vector, n, r, s, "Step 4: Untranspose and Reshape", taskid, comm);

    // Step 5: Sort columns
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_small");
    std::sort(local_vector.begin(), local_vector.end());
    CALI_MARK_END("comp_small");
    CALI_MARK_END("comp");
    print_matrix(local_vector, n, r, s, "Step 5: Sort columns", taskid, comm);

    // Step 6-8: expand the columns
    // parallel_shift_and_unshift(local_vector, n, r, s, taskid, comm);
}