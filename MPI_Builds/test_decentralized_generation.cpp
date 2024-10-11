#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <vector>
#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

#include "src/decentralized_generation.h"

int main (int argc, char *argv[]) {
    int pid, num_processors;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&pid);
    MPI_Comm_size(MPI_COMM_WORLD,&num_processors);
    unsigned int global_seq_size = 102;

    if (num_processors >= 2) {
        std::vector<unsigned int> local_seq;

        /* test case 1: sorted */
        local_seq.clear();
        local_seq = decentralized_generation(pid, num_processors, global_seq_size, 0);
        for (unsigned int i = 0; i < num_processors; ++i) {
            MPI_Barrier(MPI_COMM_WORLD);
            if (pid == i) {
                printf ("DEBUG [p%d]: seq = {", pid);
                for (unsigned int j = 0; j < local_seq.size(); ++j){
                    printf("%d ", local_seq.at(j));
                }
                printf("}.\n");
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        if (pid == 0) {
            printf("\n");
        }

        /* test case 2: reverse sorted */
        local_seq.clear();
        local_seq = decentralized_generation(pid, num_processors, global_seq_size, 1);
        for (unsigned int i = 0; i < num_processors; ++i) {
            MPI_Barrier(MPI_COMM_WORLD);
            if (pid == i) {
                printf ("DEBUG [p%d]: seq = {", pid);
                for (unsigned int j = 0; j < local_seq.size(); ++j){
                    printf("%d ", local_seq.at(j));
                }
                printf("}.\n");
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        if (pid == 0) {
            printf("\n");
        }

        /* test case 3: 1% pertubed */
        local_seq.clear();
        local_seq = decentralized_generation(pid, num_processors, global_seq_size, 2);
        for (unsigned int i = 0; i < num_processors; ++i) {
            MPI_Barrier(MPI_COMM_WORLD);
            if (pid == i) {
                printf ("DEBUG [p%d]: seq = {", pid);
                for (unsigned int j = 0; j < local_seq.size(); ++j){
                    printf("%d ", local_seq.at(j));
                }
                printf("}.\n");
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        if (pid == 0) {
            printf("\n");
        }

        /* test case 4: random */
        local_seq.clear();
        local_seq = decentralized_generation(pid, num_processors, global_seq_size, 3);
        for (unsigned int i = 0; i < num_processors; ++i) {
            MPI_Barrier(MPI_COMM_WORLD);
            if (pid == i) {
                printf ("DEBUG [p%d]: seq = {", pid);
                for (unsigned int j = 0; j < local_seq.size(); ++j){
                    printf("%d ", local_seq.at(j));
                }
                printf("}.\n");
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        if (pid == 0) {
            printf("\n");
        }
    }

    MPI_Finalize();
    return 0;
}
