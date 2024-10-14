#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

#include "../algorithms/decentralized_generation.h"
#include "../algorithms/samplesort.h"
#include "../algorithms/sort_validation.h"

#include <math.h>
#include <stdexcept>
#include <vector>

int main (int argc, char *argv[]) {
    /* variable and MPI initialization */
    int pid, num_processors;
    std::vector<unsigned int> local_seq;
    unsigned int global_seq_size = (unsigned int) pow(2, 16);
    bool is_sorted;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&pid);
    MPI_Comm_size(MPI_COMM_WORLD,&num_processors);
    /* adiak metadata */
    adiak::init(NULL);
    adiak::launchdate(); // launch date of the job
    adiak::libraries(); // Libraries used
    adiak::cmdline(); // Command line used to launch the job
    adiak::clustername(); // Name of the cluster
    adiak::value("algorithm", "samplesort"); // The name of the algorithm you are using (e.g., "merge", "bitonic")
    adiak::value("programming_model", "mpi"); // e.g. "mpi"
    adiak::value("data_type", "unsigned int"); // The datatype of input elements (e.g., double, int, float)
    adiak::value("size_of_data_type", sizeof(unsigned int)); // sizeof(datatype) of input elements in bytes (e.g., 1, 2, 4)
    adiak::value("input_size", global_seq_size); // The number of elements in input dataset (1000)
    adiak::value("input_type", "Sorted"); // For sorting, this would be choices: ("Sorted", "ReverseSorted", "Random", "1_perc_perturbed")
    adiak::value("num_procs", num_processors); // The number of processors (MPI ranks)
    adiak::value("scalability", "weak"); // The scalability of your algorithm. choices: ("strong", "weak")
    adiak::value("group_num", 25); // The number of your group (integer, e.g., 1, 10)
    adiak::value("implementation_source", "handwritten"); // Where you got the source code of your algorithm. choices: ("online", "ai", "handwritten").
    /* caliper initialization */
    CALI_CXX_MARK_FUNCTION;
    cali::ConfigManager mgr;
    mgr.start();
    /* caliper regions */
    const char* data_init_runtime = "data_init_runtime";
    const char* sort_runtime = "sort_runtime";
    const char* comm = "comm";
    const char* comm_gather_sample = "comm_gather_sample";
    const char* comm_bcast_pivots = "comm_bcast_pivots";
    const char* comm_exchange_buckets = "comm_exchange_buckets";
    const char* comp = "comp";
    const char* comp_sampling_local = "comp_sampling_local";
    const char* comp_determine_pivots = "comp_determine_pivots";
    const char* comp_local_to_buckets = "comp_local_to_buckets";
    const char* comp_sort_recv_buf = "comp_sort_recv_buf";
    const char* correctness_check = "correctness_check";

    if (num_processors >= 2){
        /* test case: sorted */
        // sequence generation
        local_seq.clear();
        CALI_MARK_BEGIN(data_init_runtime);
        local_seq = decentralized_generation(pid, num_processors, global_seq_size, 0);
        CALI_MARK_END(data_init_runtime);
        // for (unsigned int i = 0; i < num_processors; ++i) {
        //     MPI_Barrier(MPI_COMM_WORLD);
        //     if (pid == i) {
        //         printf ("DEBUG [p%d]: local_seq = {", pid);
        //         for (unsigned int j = 0; j < local_seq.size(); ++j){
        //             printf("%u ", local_seq.at(j));
        //         }
        //         printf("}.\n");
        //     }
        //     MPI_Barrier(MPI_COMM_WORLD);
        // }
        // MPI_Barrier(MPI_COMM_WORLD);
        // sorting
        try {
            CALI_MARK_BEGIN(sort_runtime);
            samplesort(local_seq, pid, num_processors, MPI_COMM_WORLD, num_processors);
            CALI_MARK_END(sort_runtime);
        } catch(const std::invalid_argument& error) {
            printf(error.what());
            MPI_Abort(MPI_COMM_WORLD, 13);
            exit(1);
        }
        // sort validation
        try {
            CALI_MARK_BEGIN(correctness_check);
            is_sorted = sort_validation(local_seq, pid, num_processors, MPI_COMM_WORLD);
            CALI_MARK_END(correctness_check);
        } catch (const std::invalid_argument& error) {
            printf(error.what());
            MPI_Abort(MPI_COMM_WORLD, 13);
            exit(1);
        }
        if (pid == 0) {
            printf ("DEBUG [p%d]: is_sorted = %d\n\n", pid, is_sorted);
        }
    }
    // cleanup
    mgr.stop();
    mgr.flush();
    MPI_Finalize();
    return 0;
}
