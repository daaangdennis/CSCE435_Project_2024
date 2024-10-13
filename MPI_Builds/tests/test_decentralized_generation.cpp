#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>
#include "../algorithms/decentralized_generation.h"

// Initialize MPI and return processor rank and size
void initialize_mpi(int &pid, int &num_processors, int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processors);
}

// Generate and print the local sequence for a given test case
void run_test_case(int pid, int num_processors, unsigned int global_seq_size, int test_case) {
    std::vector<unsigned int> local_seq = decentralized_generation(pid, num_processors, global_seq_size, test_case);

    for (unsigned int i = 0; i < num_processors; ++i) {
        MPI_Barrier(MPI_COMM_WORLD);
        if (pid == i) {
            printf("DEBUG [p%d]: seq = {", pid);
            for (unsigned int j = 0; j < local_seq.size(); ++j) {
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

// Run all test cases
void run_all_test_cases(int pid, int num_processors, unsigned int global_seq_size) {
    // Test case 1: sorted
    run_test_case(pid, num_processors, global_seq_size, 0);

    // Test case 2: reverse sorted
    run_test_case(pid, num_processors, global_seq_size, 1);

    // Test case 3: 1% perturbed
    run_test_case(pid, num_processors, global_seq_size, 2);

    // Test case 4: random
    run_test_case(pid, num_processors, global_seq_size, 3);

    // Test case 5: more processors than global sequence
    run_test_case(pid, num_processors, 0, 2);
}

int main(int argc, char *argv[]) {
    int pid, num_processors;
    unsigned int global_seq_size = 102;

    initialize_mpi(pid, num_processors, argc, argv);

    if (num_processors >= 2) {
        run_all_test_cases(pid, num_processors, global_seq_size);
    }

    MPI_Finalize();
    return 0;
}

