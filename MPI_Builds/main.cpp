#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

// algorithms
#include "algorithms/bitonicsort.h"
#include "algorithms/columnsort.h"
#include "algorithms/mergesort.h"
#include "algorithms/radixsort.h"
#include "algorithms/samplesort.h"
#include "algorithms/decentralized_generation.h"
#include "algorithms/sort_validation.h"

// temporarily used for data generation
#include <algorithm>
#include <random>

#define MASTER 0 /* taskid of first task */
#define DEBUG 1 /* print debugging */
/*
example debug usage:

#if DEBUG
printf("will print if debug is true")
#endif
*/

int main(int argc, char *argv[])
{
    CALI_CXX_MARK_FUNCTION;
    const char* main = "main";                              // main function
    const char* data_init_runtime = "data_init_runtime";    // generating data during the program
    const char* comm = "comm";                              // all communication related functions
    const char* comm_small = "comm_small";                  // send/broadcast few values
    const char* comm_large = "comm_large";                  // sending all local values
    const char* comp = "comp";                              // all computation related functions
    const char* comp_small = "comp_small";                  // sorting a few values
    const char* comp_large = "comp_large";                  // sorting values in the array

    if (argc != 4)
    {
        printf("Usage: sbatch mpi.grace_job <arr size> <arr type> <algorithm>\n");
        return 1;
    }

    unsigned int array_size;
    std::string array_type, algorithm;

    // try to parse the variables
    try {
        array_size = round((pow(2, std::stoi(argv[1]))));
        array_type = std::string(argv[2]);
        algorithm = std::string(argv[3]);
    } catch (const std::exception &e) {
        printf("Invalid args.\n");
        return 1;
    }
    
    int taskid, numtasks;

    // init mpi
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    if (numtasks < 2)
    {
        printf("Need at least two MPI tasks. Quitting...\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
        return 1;
    }
    
    // init caliper, use config in mpi.grace_job
    cali_init();
    cali::ConfigManager mgr;
    mgr.start();

    // start main region
    // std::vector<unsigned int> main_vector(array_size, 0);
    CALI_MARK_BEGIN(main);

    // MPI_Comm worker_comm;
    // MPI_Comm_split(MPI_COMM_WORLD, taskid == MASTER ? MPI_UNDEFINED : 0, taskid, &worker_comm);

    // /********************** MASTER **************************/
    // if (taskid == MASTER)
    // {
    //     #if DEBUG
    //     printf("arraysize: %u\n arraytype: %s\n algo: %s\n",
    //         array_size,
    //         array_type.c_str(),
    //         algorithm.c_str());
    //     #endif
    // }
    
    #if DEBUG
    printf("arraysize: %u\n arraytype: %s\n algo: %s\n",
        array_size,
        array_type.c_str(),
        algorithm.c_str());
    #endif
    
    /****************** generate the data (temp) ***********************/
    // main_vector={1,2,3,4};

    // if (taskid==0){
    //     for (unsigned int element : main_vector) {
    //         printf("&d",element);
    //     }
    // }

    std::vector<unsigned int> main_vector;
    /********************** generate the data *****************************/
    if (array_type=="sorted"){
        main_vector = decentralized_generation(taskid, numtasks, array_size, 0);
    }
    else if (array_type=="reverse"){
        main_vector = decentralized_generation(taskid, numtasks, array_size, 1);
    }
    else if (array_type=="perturbed"){
        main_vector = decentralized_generation(taskid, numtasks, array_size, 2);
    }
    else if (array_type=="random"){
        main_vector = decentralized_generation(taskid, numtasks, array_size, 3);
    }
    else{
        printf("Invalid array type");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    /********************** sort the data *****************************/
    if (algorithm=="bitonic"){
        bitonic_sort(main_vector, taskid, numtasks, MPI_COMM_WORLD);
    }
    else if (algorithm=="column"){
        //column_sort(main_vector, array_size, worker_comm);
    }
    else if (algorithm=="merge"){
        //merge_sort(main_vector, array_size, worker_comm);
    }
    else if (algorithm=="radix"){
        //radix_sort(main_vector, &worker_comm, argv);
    }
    else if (algorithm=="sample"){
        //sample_sort(main_vector, array_size, worker_comm);
    }
    else{
        printf("Invalid algorithm");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // validate it
    bool is_everyone_sorted = sort_validation(main_vector, taskid, numtasks, MPI_COMM_WORLD);
    for (unsigned int i = 0; i < numtasks; ++i){
        MPI_Barrier(MPI_COMM_WORLD);
        if (i == taskid){
            printf("DEBUG [p%d]: is_everyone_sorted = %d.\n", taskid, is_everyone_sorted);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // end main region
    CALI_MARK_END(main);

    // flush Caliper output before finalizing MPI
    mgr.stop(); mgr.flush();
    MPI_Finalize();

    return 0;
}

