#include "bitonicsort.h"


void bitonic_sort(std::vector<unsigned int>& local_seq, const int& taskid, const int& numtasks, const MPI_Comm& comm){
    
    // Debug loop to check initial sequence
    for (unsigned int i = 0; i < numtasks; ++i) {
        MPI_Barrier(comm);
        if (taskid == i) {
            printf ("DEBUG [p%d]: Entered with local_seq = {", taskid);
            for (unsigned long long j = 0; j < local_seq.size(); ++j) {
                printf("%u ", local_seq.at(j));
            }
            printf("}.\n");
        }
        MPI_Barrier(comm);
    }

    // // Perform individual sorts based on taskid to prepare for merge
    // if ((taskid % 2) == 0){
    //     std::sort(local_seq.begin(), local_seq.end());
    // }
    // else{
    //     std::sort(local_seq.begin(), local_seq.end(), std::greater<unsigned int>());
    // }

    // Debug loop to check sequence after sorts
    for (unsigned int i = 0; i < numtasks; ++i) {
        MPI_Barrier(comm);
        if (taskid == i) {
            printf ("DEBUG [p%d]: After sort, local_seq = {", taskid);
            for (unsigned long long j = 0; j < local_seq.size(); ++j) {
                printf("%u ", local_seq.at(j));
            }
            printf("}.\n");
        }
        MPI_Barrier(comm);
    }

    int local_n = local_seq.size();

    // Bitonic merge
    // for (int step = 1; step < numtasks; step *= 2) {
    //     for (int phase = step; phase > 0; phase /= 2) {
    //         int partner = taskid ^ (1 << phase);
    //         printf ("DEBUG [p%d]: partner = %d\n", taskid, partner);

    //         if (partner < numtasks) {
    //             // Decide the direction of comparison
    //             bool direction = ((taskid / step) % 2 == 0);
                
    //             // Perform the exchange and comparison
    //             std::vector<unsigned int> recv_vec(local_n);

    //             // Exchange data with the partner
    //             MPI_Sendrecv(local_seq.data(), local_n, MPI_UNSIGNED, partner, 0,
    //                         recv_vec.data(), local_n, MPI_UNSIGNED, partner, 0,
    //                         comm, MPI_STATUS_IGNORE);

    //             // Compare and swap elements with the partner based on direction (ascending or descending)
    //             for (int i = 0; i < local_n; ++i) {
    //                 if ((direction && local_seq[i] > recv_vec[i]) || (!direction && local_seq[i] < recv_vec[i])) {
    //                     std::swap(local_seq[i], recv_vec[i]);
    //                 }
    //             }
    //         }
    //         MPI_Barrier(comm);  // Synchronize processes
    //         for (unsigned int i = 0; i < numtasks; ++i) {
    //             MPI_Barrier(comm);
    //             if (taskid == i) {
    //                 printf ("DEBUG [p%d]: Step %d, local_seq = {", taskid, step);
    //                 for (unsigned long long j = 0; j < local_seq.size(); ++j) {
    //                     printf("%u ", local_seq.at(j));
    //                 }
    //                 printf("}.\n");
    //             }
    //             MPI_Barrier(comm);
    //         }
    //     }
    // }
    std::sort(local_seq.begin(), local_seq.end());
    for (int step = 2; step <= numtasks; step <<= 1) {
        for (int phase = step >> 1; phase > 0; phase >>= 1) {
            // Determine partner
            auto partnerid = taskid ^ phase;
            printf ("DEBUG [p%d]: partner = %d\n", taskid, partnerid);

            if (partnerid < numtasks) {
                const auto direction = ((taskid & step) == 0) ? 0 : 1;

                auto partner_seq = std::vector<unsigned int>(local_n);
                MPI_Sendrecv(local_seq.data(), local_n, MPI_UNSIGNED, partnerid, 0,
                                partner_seq.data(), local_n, MPI_UNSIGNED, partnerid, 0,
                                comm, MPI_STATUS_IGNORE);

                auto merged_seq = std::vector<unsigned int>(local_n * 2);
                std::merge(local_seq.begin(), local_seq.end(), partner_seq.begin(), partner_seq.end(), merged_seq.begin());

                if ((direction == 0 && taskid < partnerid) || (direction == 1 && taskid > partnerid)) {
                    std::copy(merged_seq.begin(), merged_seq.begin() + local_n, local_seq.begin());
                } else {
                    std::copy(merged_seq.begin() + local_n, merged_seq.end(), local_seq.begin());
                }
            }
            MPI_Barrier(comm);
            for (unsigned int i = 0; i < numtasks; ++i) {
                MPI_Barrier(comm);
                if (taskid == i) {
                    printf ("DEBUG [p%d]: Step %d, local_seq = {", taskid, step);
                    for (unsigned long long j = 0; j < local_seq.size(); ++j) {
                        printf("%u ", local_seq.at(j));
                    }
                    printf("}.\n");
                }
                MPI_Barrier(comm);
            }
        }
    }

    // Debug loop to check sequence after merge
    for (unsigned int i = 0; i < numtasks; ++i) {
        MPI_Barrier(comm);
        if (taskid == i) {
            printf ("DEBUG [p%d]: After merge, local_seq = {", taskid);
            for (unsigned long long j = 0; j < local_seq.size(); ++j) {
                printf("%u ", local_seq.at(j));
            }
            printf("}.\n");
        }
        MPI_Barrier(comm);
    }
}