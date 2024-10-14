#include "bitonicsort.h"


void bitonic_sort(std::vector<unsigned int>& local_seq, const int& taskid, const int& numtasks, const MPI_Comm& comm){
    
    // Debug loop to check initial sequence
    // for (unsigned int i = 0; i < numtasks; ++i) {
    //     MPI_Barrier(comm);
    //     if (taskid == i) {
    //         printf ("DEBUG [p%d]: Entered with local_seq = {", taskid);
    //         for (unsigned long long j = 0; j < local_seq.size(); ++j) {
    //             printf("%u ", local_seq.at(j));
    //         }
    //         printf("}.\n");
    //     }
    //     MPI_Barrier(comm);
    // }

    // Perform individual sorts to prepare for merge
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_sort_local");
    std::sort(local_seq.begin(), local_seq.end());
    CALI_MARK_END("comp_sort_local");
    CALI_MARK_END("comp");

    // Debug loop to check sequence after sorts
    // for (unsigned int i = 0; i < numtasks; ++i) {
    //     MPI_Barrier(comm);
    //     if (taskid == i) {
    //         printf ("DEBUG [p%d]: After sort, local_seq = {", taskid);
    //         for (unsigned long long j = 0; j < local_seq.size(); ++j) {
    //             printf("%u ", local_seq.at(j));
    //         }
    //         printf("}.\n");
    //     }
    //     MPI_Barrier(comm);
    // }

    int local_n = local_seq.size();

    // Bitonic merge
    for (int step = 2; step <= numtasks; step <<= 1) {
        for (int phase = step >> 1; phase > 0; phase >>= 1) {
            // Determine partner
            int partnerid = taskid ^ phase;
            //printf ("DEBUG [p%d]: partner = %d\n", taskid, partnerid);

            if (partnerid < numtasks) {
                int direction = ((taskid & step) == 0) ? 0 : 1;

                auto partner_seq = std::vector<unsigned int>(local_n);

                CALI_MARK_BEGIN("comm");
                CALI_MARK_BEGIN("comm_small");
                MPI_Sendrecv(local_seq.data(), local_n, MPI_UNSIGNED, partnerid, 0,
                                partner_seq.data(), local_n, MPI_UNSIGNED, partnerid, 0,
                                comm, MPI_STATUS_IGNORE);
                CALI_MARK_END("comm_small");
                CALI_MARK_END("comm");

                auto merged_seq = std::vector<unsigned int>(local_n * 2);
                CALI_MARK_BEGIN("comp");
                CALI_MARK_BEGIN("comp_sort_combined");
                std::merge(local_seq.begin(), local_seq.end(), partner_seq.begin(), partner_seq.end(), merged_seq.begin());
                CALI_MARK_END("comp_sort_combined");
                CALI_MARK_END("comp");

                if ((!direction && taskid < partnerid) || (direction && taskid > partnerid)) {
                    std::copy(merged_seq.begin(), merged_seq.begin() + local_n, local_seq.begin());
                } else {
                    std::copy(merged_seq.begin() + local_n, merged_seq.end(), local_seq.begin());
                }
            }
            CALI_MARK_BEGIN("comm");
            MPI_Barrier(comm);
            CALI_MARK_END("comm");
            // for (unsigned int i = 0; i < numtasks; ++i) {
            //     MPI_Barrier(comm);
            //     if (taskid == i) {
            //         printf ("DEBUG [p%d]: Step %d, local_seq = {", taskid, step);
            //         for (unsigned long long j = 0; j < local_seq.size(); ++j) {
            //             printf("%u ", local_seq.at(j));
            //         }
            //         printf("}.\n");
            //     }
            //     MPI_Barrier(comm);
            // }
        }
    }

    // Debug loop to check sequence after merge
    // for (unsigned int i = 0; i < numtasks; ++i) {
    //     MPI_Barrier(comm);
    //     if (taskid == i) {
    //         printf ("DEBUG [p%d]: After merge, local_seq = {", taskid);
    //         for (unsigned long long j = 0; j < local_seq.size(); ++j) {
    //             printf("%u ", local_seq.at(j));
    //         }
    //         printf("}.\n");
    //     }
    //     MPI_Barrier(comm);
    // }
}