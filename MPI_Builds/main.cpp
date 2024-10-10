#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <cmath>
#include <vector>

#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

#define MASTER 0 /* taskid of first task */

int main(int argc, char *argv[])
{

    if (argc != 4)
    {
        printf("Input size: <input_size> Input type: <input_type> Algorithm: <algorithm>");
        exit(1);
    }

    unsigned int input_size = round((pow(2, std::stoi(argv[1]))));
    std::string input_type(argv[2]);
    std::string algorithm(argv[3]);

    int numtasks, taskid;

    // int numworkers = numtasks - 1;
    // vector<unsigned int> worker_vector((unsigned int)(input_size / numworkers), 0);
    // REFERENCE THIS FOR ALGORITHMS

    vector<unsigned int> main_vector(input_size, 0);
    const char *data_init_runtime = 'data_init_runtime';

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    if (numtasks < 2)
    {
        printf("Need at least two MPI tasks. Quitting...\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
        exit(1);
    }

    cali::ConfigManager mgr;
    mgr.start();

    MPI_Comm_split(MPI_COMM_WORLD, taskid == MASTER ? MPI_UNDEFINED : 0, taskid, &worker_comm);

    if (taskid == MASTER)
    {
        CALI_MARK_BEGIN(data_init_runtime);

        MPI_Comm_rank(worker_comm, &worker_rank);

        unsigned int perturbed_index = round(input_size * 0.01);

        if (input_size == "sorted")
        {
            for (unsigned int i = 0; i < input_size; i++)
            {
                main_vector[i] = i;
            }
        }
        if (input_size == "reverse")
        {
            for (unsigned int i = 0; i < input_size; i++)
            {
                main_vector[i] = input_size - i;
            }
        }
        if (input_size == "random")
        {
            for (unsigned int i = 0; i < input_size; i++)
            {
                main_vector[i] = rand() % input_size;
            }
        }
        if (input_size == "perturbed")
        {
            for (unsigned int i = 0; i < input_size - perturbed_index; i++)
            {
                main_vector[i] = i;
            }
            for (unsigned int i = input_size - perturbed_index; i < input_size; i++)
            {
                main_vector[i] = rand() % input_size;
            }
        }

        CALI_MARK_END(data_init_runtime);

        // implement based on how call you function

        // if (algorithm == "bitonic")
        // {
        // }
        // if (algorithm == "merge")
        // {
        // }
        // if (algorithm == "sample")
        // {
        // }
        // if (algorithm == "radix")
        // {
        // }
        // if (algorithm == "column")
        // {
        // }

        return 0;
    }
}