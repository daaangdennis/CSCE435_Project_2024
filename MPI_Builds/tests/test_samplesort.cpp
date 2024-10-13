#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <vector>
#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

#include "../algorithms/decentralized_generation.h"
#include "../algorithms/samplesort.h"
#include "../algorithms/sort_validation.h"

int main (int argc, char *argv[]) {
    int pid, num_processors;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&pid);
    MPI_Comm_size(MPI_COMM_WORLD,&num_processors);
    std::vector<unsigned int> local_seq;
    unsigned int global_seq_size = 102;
    bool is_sorted;
    
    if (num_processors >= 2){
        /* test case 1: sorted */
        // sequence generation
        local_seq.clear();
        local_seq = decentralized_generation(pid, num_processors, global_seq_size, 0);
        for (unsigned int i = 0; i < num_processors; ++i) {
            MPI_Barrier(MPI_COMM_WORLD);
            if (pid == i) {
                printf ("DEBUG [p%d]: local_seq = {", pid);
                for (unsigned int j = 0; j < local_seq.size(); ++j){
                    printf("%u ", local_seq.at(j));
                }
                printf("}.\n");
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        // sorting
        samplesort(local_seq, pid, num_processors, MPI_COMM_WORLD, num_processors);
        // sort validation
        is_sorted = sort_validation(local_seq, pid, num_processors, MPI_COMM_WORLD);
        if (pid == 0) {
            printf ("DEBUG [p%d]: is_sorted = %d\n\n", pid, is_sorted);
        }

        /* test case 2: reverse sorted */
        // sequence generation
        local_seq.clear();
        local_seq = decentralized_generation(pid, num_processors, global_seq_size, 1);
        for (unsigned int i = 0; i < num_processors; ++i) {
            MPI_Barrier(MPI_COMM_WORLD);
            if (pid == i) {
                printf ("DEBUG [p%d]: local_seq = {", pid);
                for (unsigned int j = 0; j < local_seq.size(); ++j){
                    printf("%u ", local_seq.at(j));
                }
                printf("}.\n");
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        // sorting
        samplesort(local_seq, pid, num_processors, MPI_COMM_WORLD, num_processors);
        // sort validation
        is_sorted = sort_validation(local_seq, pid, num_processors, MPI_COMM_WORLD);
        if (pid == 0) {
            printf ("DEBUG [p%d]: is_sorted = %d\n\n", pid, is_sorted);
        }

        /* test case 3: 1% pertubed */
        // sequence generation
        local_seq.clear();
        local_seq = decentralized_generation(pid, num_processors, global_seq_size, 2);
        for (unsigned int i = 0; i < num_processors; ++i) {
            MPI_Barrier(MPI_COMM_WORLD);
            if (pid == i) {
                printf ("DEBUG [p%d]: local_seq = {", pid);
                for (unsigned int j = 0; j < local_seq.size(); ++j){
                    printf("%u ", local_seq.at(j));
                }
                printf("}.\n");
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        // sorting
        samplesort(local_seq, pid, num_processors, MPI_COMM_WORLD, num_processors);
        // sort validation
        is_sorted = sort_validation(local_seq, pid, num_processors, MPI_COMM_WORLD);
        if (pid == 0) {
            printf ("DEBUG [p%d]: is_sorted = %d\n\n", pid, is_sorted);
        }

        /* test case 4: random */
        // sequence generation
        local_seq.clear();
        local_seq = decentralized_generation(pid, num_processors, global_seq_size, 3);
        for (unsigned int i = 0; i < num_processors; ++i) {
            MPI_Barrier(MPI_COMM_WORLD);
            if (pid == i) {
                printf ("DEBUG [p%d]: local_seq = {", pid);
                for (unsigned int j = 0; j < local_seq.size(); ++j){
                    printf("%u ", local_seq.at(j));
                }
                printf("}.\n");
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        // sorting
        samplesort(local_seq, pid, num_processors, MPI_COMM_WORLD, num_processors);
        // sort validation
        is_sorted = sort_validation(local_seq, pid, num_processors, MPI_COMM_WORLD);
        if (pid == 0) {
            printf ("DEBUG [p%d]: is_sorted = %d\n\n", pid, is_sorted);
        }
    }
    
    MPI_Finalize();
    return 0;
}
