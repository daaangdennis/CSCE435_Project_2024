#include "bitonicsort.h"

int bitonic_sort(std::vector<unsigned int>& local_seq, const int& taskid, const int& numtasks, const MPI_Comm& comm){
    for (unsigned int i = 0; i < numtasks; ++i) {
        MPI_Barrier(comm);
        if (taskid == i) {
            printf ("DEBUG [p%d]: Entered with local_sample = {", taskid);
            for (unsigned long long j = 0; j < local_seq.size(); ++j) {
                printf("%u ", local_seq.at(j));
            }
            printf("}.\n");
        }
        MPI_Barrier(comm);
    }
    return 0;
}