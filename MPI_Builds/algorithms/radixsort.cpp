#include "radixsort.h"

int radix_sort(std::vector<unsigned int> main_vector, MPI_Comm* worker_comm, char** argv) {
    CALI_CXX_MARK_FUNCTION;

    // parse argv
    unsigned int array_size;
    std::string array_type, algorithm;

    try {
        array_size = round((pow(2, std::stoi(argv[1]))));
        array_type = std::string(argv[2]);
        algorithm = std::string(argv[3]);
    } catch (const std::exception &e) {
        printf("Invalid args.\n");
        return 1;
    }

    int taskid, numtasks; 
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    adiak::init(NULL);
    adiak::launchdate();    // launch date of the job
    adiak::libraries();     // Libraries used
    adiak::cmdline();       // Command line used to launch the job
    adiak::clustername();   // Name of the cluster
    adiak::value("algorithm", "radix"); // The name of the algorithm you are using (e.g., "merge", "bitonic")
    adiak::value("programming_model", "mpi"); // e.g. "mpi"
    adiak::value("data_type", "int"); // The datatype of input elements (e.g., double, int, float)
    adiak::value("size_of_data_type", sizeof(int)); // sizeof(datatype) of input elements in bytes (e.g., 1, 2, 4)
    adiak::value("input_size", array_size); // The number of elements in input dataset (1000)
    adiak::value("input_type", array_type); // For sorting, this would be choices: ("Sorted", "ReverseSorted", "Random", "1_perc_perturbed")
    adiak::value("num_procs", numtasks); // The number of processors (MPI ranks)
    adiak::value("scalability", "temp"); // The scalability of your algorithm. choices: ("strong", "weak")
    adiak::value("group_num", 25); // The number of your group (integer, e.g., 1, 10)
    adiak::value("implementation_source", "temp"); // Where you got the source code of your algorithm. choices: ("online", "ai", "handwritten").

    return 0;
}
