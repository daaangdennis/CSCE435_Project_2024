#include "sort_validation.h"

/*
    - input
        - local_seq: locally stored sequence
        - pid: processor id
        - num_processors: number of processors running in parallel
        - comm: communicator
    - output: true if the global sequence is sorted, false otherwise
*/
bool sort_validation(const std::vector<unsigned int>& local_seq, const int& pid, const int& num_processors, const MPI_Comm& comm) {
    /* check local sequence is sorted */
    bool local_seq_sorted = std::is_sorted(local_seq.begin(),local_seq.end());
    /* 
        compare my last value against right neighbor's first value
        - each processor should send their first value to the left neighbor
            - the tag in send and recv should represent who send the value
        - neighbor comparison passes if my last value is smaller or equal to received value
    */
    unsigned int my_first = local_seq.at(0);
    unsigned int my_last = local_seq.at(local_seq.size() - 1);
    unsigned int recv_val = 0;
    MPI_Status status;
    if (pid != 0) {
        if ((pid % 2) == 0) {
            // even processors (except processor 0) send first element left
            MPI_Send(&my_first, 1, MPI_UNSIGNED, pid - 1, pid, comm);
        } else if (pid != (num_processors - 1)) {
            // odd processors (except the last processor) receive
            MPI_Recv(&recv_val, 1, MPI_UNSIGNED, pid + 1, pid + 1, comm, &status);
        }
    }
    if ((pid % 2) != 0){ 
        // odd processors send first element left
        MPI_Send(&my_first, 1, MPI_UNSIGNED, pid - 1, pid, comm);
    } else if (pid != (num_processors - 1)) { 
        // even processors (except the last processor) recv
        MPI_Recv(&recv_val, 1, MPI_UNSIGNED, pid + 1, pid + 1, comm, &status);
    }
    bool neighbor_comparison_passed = ((pid == (num_processors - 1)) ? true : (my_last <= recv_val));
    /* I pass if I'm sorted locally and I passed neighbor comparison test*/
    bool my_result = (local_seq_sorted && neighbor_comparison_passed);
    printf ("DEBUG [p%d]: local_seq_sorted = %d, neighbor_comparison_passed = %d.\n", 
        pid, local_seq_sorted, neighbor_comparison_passed
    );
    /* 
        notify other processors my status, and get the final result
        - reduce to a single bool value on processor 0
        - processor 0 has the responsibility of broadcasting the final result
    */
    bool final_result = false;
    MPI_Reduce(&my_result, &final_result, 1, MPI_C_BOOL, MPI_LAND, 0, comm);
    MPI_Bcast(&final_result, 1, MPI_C_BOOL, 0, comm);
    return final_result;
}
