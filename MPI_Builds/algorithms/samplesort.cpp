#include "samplesort.h"

/*
    - input
        - local_seq: locally stored sequence
        - pid: processor id
        - num_processors: number of processors running in parallel
        - comm: communicator
        - k: oversampling factor (how many number to sample from the local sequence)
    - output: 
        - modifies the local sequence on each processor
        - ensuring concatenated local sequences result in a sorted global sequence
    - assumptions:
        - more than 1 processor running in parallel
        - 64-bit system (i.e. sizeof(size_t) == sizeof(unsigned long long))
        - 0 < K <= local_seq.size()
*/
void samplesort(
    std::vector<unsigned int>& local_seq, 
    const int& pid, const int& num_processors, const MPI_Comm& comm,
    const unsigned long long& K
) {
    /* assumptions check */
    if ((num_processors <= 1) || (sizeof(size_t) != sizeof(unsigned long long)) || (K <= 0) || (K > local_seq.size())) {
        throw std::invalid_argument("assumptions not met, please see documentation");
    }
    /* take K samples from local sequence */
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_sampling_local");
    std::vector<unsigned int> local_sample(K, 0);
    if (K == 1) {
        local_sample.at(0) = local_seq.at(local_seq.size() / 2);
    } else if (K >= 2) {
        local_sample.at(0) = local_seq.at(0);
        local_sample.at(local_sample.size() - 1) = local_seq.at(local_seq.size() - 1);
        if (K > 2){
            for (unsigned long long i = 1; i < local_sample.size() - 1; ++i) {
                local_sample.at(i) = local_seq.at(i * ((local_seq.size() - 2) / (K - 1) + 1));
            }
        }  
    }
    CALI_MARK_END("comp_sampling_local");
    CALI_MARK_END("comp");
    // for (unsigned int i = 0; i < num_processors; ++i) {
    //     MPI_Barrier(comm);
    //     if (pid == i) {
    //         printf ("DEBUG [p%d]: local_sample = {", pid);
    //         for (unsigned long long j = 0; j < local_sample.size(); ++j) {
    //             printf("%u ", local_sample.at(j));
    //         }
    //         printf("}.\n");
    //     }
    //     MPI_Barrier(comm);
    // }
    /* gather samples to processor 0 */
    std::vector<unsigned int> gathered_samples(K * num_processors, 0);
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_gather_sample");
    MPI_Gather(&local_sample[0], K, MPI_UNSIGNED, &gathered_samples[0], K, MPI_UNSIGNED, 0, comm);
    CALI_MARK_END("comm_gather_sample");
    CALI_MARK_END("comm");
    /* processor 0 determine pivots*/
    std::vector<unsigned int> pivots(num_processors - 1, 0);
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_determine_pivots");
    if (pid == 0) {
        /* sort */
        std::sort(gathered_samples.begin(), gathered_samples.end());
        // printf ("DEBUG [p%d]: gathered_samples = {", pid);
        // for (unsigned long long i = 0; i < gathered_samples.size(); ++i) {
        //     printf("%u ", gathered_samples.at(i));
        // }
        // printf("}.\n");
        /* choose pivots */
        for (unsigned long long i = 0; i < pivots.size(); ++i) {
            pivots.at(i) = gathered_samples.at((i + 1) * (gathered_samples.size() / num_processors));
        }
        // printf ("DEBUG [p%d]: pivots = {", pid);
        // for (unsigned long long i = 0; i < pivots.size(); ++i) {
        //     printf("%u ", pivots.at(i));
        // }
        // printf("}.\n");
    }
    CALI_MARK_END("comp_determine_pivots");
    CALI_MARK_END("comp");
    /* bcast pivots */
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_bcast_pivots");
    MPI_Bcast(&pivots[0], pivots.size(), MPI_UNSIGNED, 0, comm);
    CALI_MARK_END("comm_bcast_pivots");
    CALI_MARK_END("comm");
    /* 
        split local sequence into buckets
        - idea: use sorted local sequence and pointers to ensure memory efficiency
    */
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_local_to_buckets");
    std::sort(local_seq.begin(), local_seq.end());
    std::vector<std::pair<unsigned int *, unsigned long long>> buckets(
        num_processors, std::pair<unsigned int *, unsigned long long>(nullptr, 0)
    ); // [(begin ptr, num values)]
    for (unsigned long long i = 0; i < buckets.size(); ++i){
        std::vector<unsigned int>::iterator bucket_start, bucket_end;
        unsigned long long num_vals_in_bucket;
        if (i == 0) {
            bucket_start = local_seq.begin();
            bucket_end = std::lower_bound(local_seq.begin(), local_seq.end(), pivots.at(0));
        } else if (i == (num_processors - 1)) {
            bucket_start = std::lower_bound(local_seq.begin(), local_seq.end(), pivots.at(pivots.size() - 1));
            bucket_end = local_seq.end();
        } else {
            bucket_start = std::lower_bound(local_seq.begin(), local_seq.end(), pivots.at(i - 1));
            bucket_end = std::lower_bound(local_seq.begin(), local_seq.end(), pivots.at(i));
        }
        buckets.at(i).second = bucket_end - bucket_start;
        buckets.at(i).first = (buckets.at(i).second > 0) ? &(*bucket_start) : nullptr;
    }
    CALI_MARK_END("comp_local_to_buckets");
    CALI_MARK_END("comp");
    // for (unsigned int i = 0; i < num_processors; ++i) {
    //     MPI_Barrier(comm);
    //     if (pid == i){
    //         for (unsigned int j = 0; j < buckets.size(); ++j) {
    //             printf("DEBUG [p%d]: buckets.at(%u) = {", pid, j);
    //             for (unsigned long long k = 0; k < buckets.at(j).second; ++k){
    //                 printf("%u ", *(buckets.at(j).first + k));
    //             }
    //             printf("}.\n");
    //         }
    //     }
    //     MPI_Barrier(comm);
    // }
    /* exchange buckets */
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_exchange_buckets");
    std::vector<int> recv_counts(num_processors, 0), recv_displacements(num_processors, 0);
    std::vector<unsigned int> recv_buf;
    for (unsigned int i = 0; i < num_processors; ++i){
        // gather the i-th bucket metadata to the i-th processor
        int bucket_size = (int) buckets.at(i).second;
        MPI_Gather(&bucket_size, 1, MPI_INT, &recv_counts[0], 1, MPI_INT, i, comm);
        // i-th processor configures for recv
        if (pid == i){
            // resize recv_buf
            unsigned long long recv_total = std::accumulate(recv_counts.begin(), recv_counts.end(), 0);
            recv_buf.resize(recv_total);
            // determine recv_displacement
            for (unsigned long long j = 0; j < recv_displacements.size(); ++j) {
                if (j == 0) {
                    recv_displacements.at(0) = 0;
                } else {
                    recv_displacements.at(j) = recv_displacements.at(j - 1) + recv_counts.at(j - 1);
                }
            }
        }
        // gather the i-th bucket to the i-th processor
        MPI_Gatherv(
            buckets.at(i).first, bucket_size, MPI_UNSIGNED,
            &recv_buf[0], &recv_counts[0], &recv_displacements[0],
            MPI_UNSIGNED, i, comm
        );
        // if (pid == i){
        //     printf("DEBUG [p%d]: recv_buf = {", pid);
        //     for (unsigned long long j = 0; j < recv_buf.size(); ++j){
        //         printf("%u ", recv_buf.at(j));
        //     }
        //     printf("}.\n");
        // }
    }
    CALI_MARK_END("comm_exchange_buckets");
    CALI_MARK_END("comm");
    /* sort recv_buf */
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_sort_recv_buf");
    std::sort(recv_buf.begin(), recv_buf.end());
    CALI_MARK_END("comp_sort_recv_buf");
    CALI_MARK_END("comp");
    /* modify local sequence */
    local_seq.clear();
    local_seq = recv_buf;
    MPI_Barrier(comm);
}
