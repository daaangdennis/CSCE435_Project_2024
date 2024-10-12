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

#define MASTER 0 /* taskid of first task */

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
    
    // check enough processes
    int numtasks, taskid;

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
    std::vector<unsigned int> main_vector(array_size, 0);
    CALI_MARK_BEGIN(main);

    MPI_Comm worker_comm;
    MPI_Comm_split(MPI_COMM_WORLD, taskid == MASTER ? MPI_UNDEFINED : 0, taskid, &worker_comm);

    /********************** MASTER **************************/
    if (taskid == MASTER)
    {
        printf("arraysize: %u\n arraytype: %s\n algo: %s\n",
               array_size,
               array_type.c_str(),
               algorithm.c_str());
    }

    // end main region
    CALI_MARK_END(main);

    // flush Caliper output before finalizing MPI
    mgr.stop(); mgr.flush();
    MPI_Finalize();

    return 0;
}

