#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <iostream>     // std::cout
#include <algorithm>    // std::is_sorted, std::prev_permutation
#include <vector>
#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

bool seq_validation(const std::vector<unsigned int>& local_seq, const int& pid, const int& num_processors, const MPI_Comm& comm) {
    /* step 1: check local sequence */
    bool am_i_sorted = std::is_sorted(local_seq.begin(),local_seq.end());
    /* step 2: check neighbors */
    bool is_neighbor_compare_good = true;
    unsigned int my_first = local_seq.at(0);
    unsigned int my_last = local_seq.at(local_seq.size() - 1);
    unsigned int recv_val = 0;
    MPI_Status status;
    /* send first element left */
    if (pid != 0){
        if ((pid % 2) == 0){ // even processors send first element left
            MPI_Send(&my_first, 1, MPI_UNSIGNED, pid - 1, pid, comm);
        } else if (pid != (num_processors - 1)) { // odd processors recv
            MPI_Recv(&recv_val, 1, MPI_UNSIGNED, pid + 1, pid + 1, comm, &status);
        }
    }
    if ((pid % 2) != 0){ // odd processors send first element left
        MPI_Send(&my_first, 1, MPI_UNSIGNED, pid - 1, pid, comm);
    } else if (pid != (num_processors - 1)) { // even processors recv
        MPI_Recv(&recv_val, 1, MPI_UNSIGNED, pid + 1, pid + 1, comm, &status);
    }
    /* check my last against recv value */
    if (pid != (num_processors - 1)){
        is_neighbor_compare_good = (recv_val >= my_last);
    }
    printf ("DEBUG [%d/%d]: am_i_sorted = %d, is_neighbor_compare_good = %d.\n", 
        pid, num_processors, am_i_sorted, is_neighbor_compare_good
    );
    bool im_i_good = (am_i_sorted && is_neighbor_compare_good);
    /* reduce to a single bool on processor 0, then processor 0 bcast to everyone else */
    bool is_everyone_sorted = false;
    MPI_Reduce(&im_i_good, &is_everyone_sorted, 1, MPI_C_BOOL, MPI_LAND, 0, comm);
    MPI_Bcast(&is_everyone_sorted, 1, MPI_C_BOOL, 0, comm);
    return is_everyone_sorted;
}

int main (int argc, char *argv[]) {
    int pid, num_processors;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&pid);
    MPI_Comm_size(MPI_COMM_WORLD,&num_processors);
    unsigned int local_seq_size = 5;
    
    if (num_processors >= 2){
        /* sorted test case */
        std::vector<unsigned int> myvector;
        for (unsigned int i = 0; i < local_seq_size; ++i){
            myvector.push_back(local_seq_size * pid + i);
        }
        for (unsigned int i = 0; i < num_processors; ++i){
            MPI_Barrier(MPI_COMM_WORLD);
            if (pid == i){
                printf ("DEBUG [%d/%d]: seq = {", pid, num_processors);
                for (unsigned int j = 0; j < local_seq_size; ++j){
                    printf("%d ", myvector.at(j));
                }
                printf("}.\n");
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
        /* run validation */
        bool is_everyone_sorted = seq_validation(myvector, pid, num_processors, MPI_COMM_WORLD);
        for (unsigned int i = 0; i < num_processors; ++i){
            MPI_Barrier(MPI_COMM_WORLD);
            if (i == pid){
                printf("DEBUG [%d/%d]: is_everyone_sorted = %d.\n", pid, num_processors, is_everyone_sorted);
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }

        /* unsorted test case */
        myvector.clear();
        for (unsigned int i = 0; i < (local_seq_size * num_processors); ++i){
            if ((i % num_processors) == pid){
                myvector.push_back(i);
            } 
        }
        for (unsigned int i = 0; i < num_processors; ++i){
            MPI_Barrier(MPI_COMM_WORLD);
            if (pid == i){
                printf ("DEBUG [%d/%d]: seq = {", pid, num_processors);
                for (unsigned int j = 0; j < local_seq_size; ++j){
                    printf("%d ", myvector.at(j));
                }
                printf("}.\n");
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
        /* run validation */
        is_everyone_sorted = seq_validation(myvector, pid, num_processors, MPI_COMM_WORLD);
        for (unsigned int i = 0; i < num_processors; ++i){
            MPI_Barrier(MPI_COMM_WORLD);
            if (i == pid){
                printf ("DEBUG [%d/%d]: is_everyone_sorted = %d.\n", pid, num_processors, is_everyone_sorted);
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
    }
}