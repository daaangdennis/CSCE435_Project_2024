#include "decentralized_generation.h"

/*
    - input
        - pid: processor id
        - num_processors: number of processors running in parallel
        - global_seq_size: the size of the global sequence
        - flag
            - 0: sorted
            - 1: reverse sorted
            - 2: 1% pertubed (1% of the global_seq_size)
            - 3: random
    - output: a local sequence of random numbers
    - assumptions
        - numbers can repeat
        - no guarantee every processor's local sequence will be the same size
*/
std::vector<unsigned int> decentralized_generation(
    const int& pid, const int& num_processors, const unsigned int& global_seq_size, const unsigned int& flag
) {
    /* calculate the size of the local sequence */
    unsigned int avg_seq_size = global_seq_size / num_processors;
    unsigned int left_over_seq_size = global_seq_size % num_processors;
    unsigned int local_seq_size = ((pid < left_over_seq_size) ? (avg_seq_size + 1) : avg_seq_size);
    /* calculate how many numbers in the local sequence should be random*/
    unsigned int num_random = 0;
    if (flag == 2) {
        unsigned int total_num_random = global_seq_size * 0.01;
        unsigned int avg_num_random = total_num_random / num_processors;
        unsigned int left_over_num_random = total_num_random % num_processors;
        num_random = ((pid < left_over_num_random) ? (avg_num_random + 1) : avg_num_random);
    } else if (flag == 3) {
        num_random = local_seq_size;
    }
    /* population local sequence based on flag */
    std::random_device rd;  // a seed source for the random number engine
    std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(0, global_seq_size);
    std::vector<unsigned int> local_seq;
    for (unsigned int i = 0; i < local_seq_size; ++i) {
        unsigned int val;
        if (i < num_random){
            val = distrib(gen);
        } else {
            if (flag == 1){
                if (pid < left_over_seq_size) {
                    val = (num_processors - left_over_seq_size) * (avg_seq_size + 1)
                        + (left_over_seq_size - 1 - pid) * avg_seq_size
                        + (local_seq_size - 1 - i);
                } else {
                    val = (num_processors - 1 - pid) * avg_seq_size
                        + (local_seq_size - 1 - i);
                }
            } else {
                if (pid < left_over_seq_size) {
                    val = pid * (avg_seq_size + 1)
                        + i;
                } else {
                    val = (left_over_seq_size) * (avg_seq_size + 1)
                        + (pid - left_over_seq_size) * avg_seq_size 
                        + i;
                }
            }
        }
        local_seq.push_back(val);
    }
    return local_seq;
}
