#include "mergesort.h"

void mergesort(std::vector<unsigned int>& local_seq, const int& taskid, const int& numtasks, const MPI_Comm& comm) 
{
    // Begin computation section
    CALI_MARK_BEGIN("comp");
    // Small computation: local sorting
    CALI_MARK_BEGIN("comp_small");
    // Each process sorts its local data
    std::sort(local_seq.begin(), local_seq.end());
    CALI_MARK_END("comp_small");
    CALI_MARK_END("comp");
    
    int num_levels = 0;
    int temp = numtasks;
    while (temp > 1) 
    {
        temp = (temp + 1) / 2;
        num_levels++;
    }

    // Iterative parallel merging
    int active = 1; // Flag indicating if the process is still active in merging
    for (int level = 0; level < num_levels; ++level) 
    {
        if (active) 
        {
            int distance = 1 << level;
            int partner;

            if ((taskid % (2 * distance)) == 0) 
            {
                partner = taskid + distance;
                if (partner < numtasks) 
                {
                    // Begin communication section
                    CALI_MARK_BEGIN("comm");
                    // Small communication: receiving data size
                    CALI_MARK_BEGIN("comm_small");
                    // Receive data size from partner
                    int recv_size;
                    MPI_Status status;
                    MPI_Recv(&recv_size, 1, MPI_INT, partner, 0, comm, &status);
                    CALI_MARK_END("comm_small");

                    // Large communication: receiving data
                    CALI_MARK_BEGIN("comm_large");
                    // Receive data from partner
                    std::vector<unsigned int> recv_data(recv_size);
                    MPI_Recv(recv_data.data(), recv_size, MPI_UNSIGNED, partner, 0, comm, &status);
                    CALI_MARK_END("comm_large");
                    CALI_MARK_END("comm");

                    // Begin computation section
                    CALI_MARK_BEGIN("comp");
                    // Large computation: merging data
                    CALI_MARK_BEGIN("comp_large");
                    // Merge local_seq and recv_data
                    std::vector<unsigned int> merged_data(local_seq.size() + recv_data.size());
                    std::merge(local_seq.begin(), local_seq.end(), recv_data.begin(), recv_data.end(), merged_data.begin());
                    local_seq.swap(merged_data);
                    CALI_MARK_END("comp_large");
                    CALI_MARK_END("comp");
                }
            } 
            else 
            {
                partner = taskid - distance;
                if (partner >= 0) 
                {
                    // Begin communication section
                    CALI_MARK_BEGIN("comm");
                    // Small communication: sending data size
                    CALI_MARK_BEGIN("comm_small");
                    // Send data size to partner
                    int send_size = local_seq.size();
                    MPI_Send(&send_size, 1, MPI_INT, partner, 0, comm);
                    CALI_MARK_END("comm_small");

                    // Large communication: sending data
                    CALI_MARK_BEGIN("comm_large");
                    // Send data to partner
                    MPI_Send(local_seq.data(), send_size, MPI_UNSIGNED, partner, 0, comm);
                    CALI_MARK_END("comm_large");
                    CALI_MARK_END("comm");
                }
                active = 0; // This process is done with merging
            }
        }
        // Begin communication section
        CALI_MARK_BEGIN("comm");
        // Small communication: synchronization barrier
        CALI_MARK_BEGIN("comm_small");
        MPI_Barrier(comm); // Synchronize processes
        CALI_MARK_END("comm_small");
        CALI_MARK_END("comm");
    }

    // Begin communication section
    CALI_MARK_BEGIN("comm");
    // Small communication: broadcasting total elements
    CALI_MARK_BEGIN("comm_small");
    // After merging, redistribute data so that each process gets its portion
    int total_elements = 0;
    if (taskid == 0)
    {
        total_elements = local_seq.size();
    }
    MPI_Bcast(&total_elements, 1, MPI_INT, 0, comm);
    CALI_MARK_END("comm_small");
    CALI_MARK_END("comm");

    // Begin computation section
    CALI_MARK_BEGIN("comp");
    // Small computation: calculating send counts and displacements
    CALI_MARK_BEGIN("comp_small");
    // Calculate send counts and displacements for scattering
    std::vector<int> send_counts(numtasks);
    std::vector<int> displs(numtasks);

    if (taskid == 0) 
    {
        int quotient = total_elements / numtasks;
        int remainder = total_elements % numtasks;
        for (int i = 0; i < numtasks; ++i) 
        {
            send_counts[i] = quotient + (i < remainder ? 1 : 0);
        }
        displs[0] = 0;
        for (int i = 1; i < numtasks; ++i) 
        {
            displs[i] = displs[i - 1] + send_counts[i - 1];
        }
    }
    CALI_MARK_END("comp_small");
    CALI_MARK_END("comp");

    // Begin communication section
    CALI_MARK_BEGIN("comm");
    // Small communication: broadcasting counts and displacements
    CALI_MARK_BEGIN("comm_small");
    // Broadcast send_counts and displacements to all processes
    MPI_Bcast(send_counts.data(), numtasks, MPI_INT, 0, comm);
    MPI_Bcast(displs.data(), numtasks, MPI_INT, 0, comm);
    CALI_MARK_END("comm_small");
    CALI_MARK_END("comm");

    // Begin communication section
    CALI_MARK_BEGIN("comm");
    // Large communication: scattering final data
    CALI_MARK_BEGIN("comm_large");
    // Each process receives its portion of the sorted data
    int recv_count = send_counts[taskid];
    std::vector<unsigned int> final_local_seq(recv_count);
    MPI_Scatterv(taskid == 0 ? local_seq.data() : nullptr, send_counts.data(), displs.data(), MPI_UNSIGNED,
                 final_local_seq.data(), recv_count, MPI_UNSIGNED, 0, comm);
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");

    // Update local_seq with the final sorted data portion
    local_seq.swap(final_local_seq);
}
