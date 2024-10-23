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
#include <algorithm>
#include <random>

#include "algorithms/decentralized_generation.h"

// global constants definitions
#define b 32           // number of bits for integer
#define g 8            // group of bits for each scan
#define N b / g        // number of passes
#define B (1 << g)     // number of buckets, 2^g
#define MASTER 0       // taskid of first task
#define DEBUG 0        // booL: print debug statements?

// MPI tags constants, offset by max bucket to avoid collisions
#define COUNTS_TAG_NUM  B + 1
#define PRINT_TAG_NUM  COUNTS_TAG_NUM + 1
#define NUM_TAG PRINT_TAG_NUM + 1

// structure encapsulating buckets with vectors of elements
struct List {
    std::vector<unsigned int> array;
};

void usage(const char* message) {
    fprintf(stderr, "Usage: sbatch mpi.grace_job [global_seq_size] [flag] [processors]\n");
    fprintf(stderr, "  [global_seq_size] - total number of elements to sort\n");
    fprintf(stderr, "  [flag] - type of array to generate (0: sorted, 1: reverse sorted, 2: 1 percent perturbed, 3: random)\n");
    fprintf(stderr, "  [processors] - number of processes to use\n");
}

// print resulting array while gathering information from all processes
void print_array(const int P, const int rank, const std::vector<unsigned int>& a, const std::vector<int>& n) {
    if (rank == 0) {
        // print array for rank 0 first
        for (int i = 0; i < n[rank]; i++) {
            printf("%u\n", a[i]);
        }
        // then receive and print from others
        for (int p = 1; p < P; p++) {
            MPI_Status stat;
            int a_size = n[p];
            std::vector<unsigned int> buff(a_size);
            MPI_Recv(buff.data(), a_size, MPI_UNSIGNED, p, PRINT_TAG_NUM, MPI_COMM_WORLD, &stat);
            for (int i = 0; i < a_size; i++) {
                printf("%u\n", buff[i]);
            }
        }
    } else {
        // if not rank 0, send your data to other processes
        MPI_Send(a.data(), n[rank], MPI_UNSIGNED, 0, PRINT_TAG_NUM, MPI_COMM_WORLD);
    }
}

bool validate_sort(const std::vector<unsigned int>& a) {
    for (size_t i = 1; i < a.size(); ++i) {
        if (a[i - 1] > a[i]) {
            return false;
        }
    }
    return true;
}

// Compute j bits which appear k bits from the right in x
unsigned bits(unsigned x, int k, int j) {
    return (x >> k) & ~(~0 << j);
}

// Modified radix_sort function to properly nest comp and comm regions
std::vector<unsigned int> radix_sort(std::vector<unsigned int>& a, std::vector<List>& buckets, const int P, const int rank, int& n) {
    CALI_MARK_BEGIN("comp");
    std::vector<std::vector<int>> count(B, std::vector<int>(P));
    std::vector<int> l_count(B);
    int l_B = B / P;
    std::vector<std::vector<int>> p_sum(l_B, std::vector<int>(P));
    CALI_MARK_END("comp");

    MPI_Request req;
    MPI_Status stat;

    for (int pass = 0; pass < N; pass++) {
        CALI_MARK_BEGIN("comp");
        CALI_MARK_BEGIN("comp_large");
        // Initialize counts and buckets
        for (int j = 0; j < B; j++) {
            count[j][rank] = 0;
            l_count[j] = 0;
            buckets[j].array.clear();
        }

        // Count items per bucket
        for (int i = 0; i < n; i++) {
            unsigned int idx = bits(a[i], pass*g, g);
            count[idx][rank]++;
            l_count[idx]++;
            buckets[idx].array.push_back(a[i]);
        }
        CALI_MARK_END("comp_large");
        CALI_MARK_END("comp");

        CALI_MARK_BEGIN("comm");
        CALI_MARK_BEGIN("comm_large");
        // One-to-all transpose
        for (int p = 0; p < P; p++) {
            if (p != rank) {
                MPI_Isend(l_count.data(), B, MPI_INT, p, COUNTS_TAG_NUM, MPI_COMM_WORLD, &req);
            }
        }

        // Receive counts
        for (int p = 0; p < P; p++) {
            if (p != rank) {
                MPI_Recv(l_count.data(), B, MPI_INT, p, COUNTS_TAG_NUM, MPI_COMM_WORLD, &stat);
                for (int i = 0; i < B; i++) {
                    count[i][p] = l_count[i];
                }
            }
        }
        CALI_MARK_END("comm_large");
        CALI_MARK_END("comm");

        CALI_MARK_BEGIN("comp");
        CALI_MARK_BEGIN("comp_small");
        // Calculate new size
        int new_size = 0;
        for (int j = 0; j < l_B; j++) {
            int idx = j + rank * l_B;
            for (int p = 0; p < P; p++) {
                p_sum[j][p] = new_size;
                new_size += count[idx][p];
            }
        }

        if (new_size > n) {
            a.resize(new_size);
        }
        CALI_MARK_END("comp_small");
        CALI_MARK_END("comp");

        CALI_MARK_BEGIN("comm");
        CALI_MARK_BEGIN("comm_large");
        // Exchange keys
        for (int j = 0; j < B; j++) {
            int p = j / l_B;
            int p_j = j % l_B;
            if (p != rank && !buckets[j].array.empty()) {
                MPI_Isend(buckets[j].array.data(), buckets[j].array.size(), MPI_UNSIGNED, p, p_j, MPI_COMM_WORLD, &req);
            }
        }

        // Receive keys
        for (int j = 0; j < l_B; j++) {
            int idx = j + rank * l_B;
            for (int p = 0; p < P; p++) {
                int b_count = count[idx][p];
                if (b_count > 0) {
                    auto dest = a.begin() + p_sum[j][p];
                    if (rank != p) {
                        std::vector<unsigned int> temp(b_count);
                        MPI_Recv(temp.data(), b_count, MPI_UNSIGNED, p, j, MPI_COMM_WORLD, &stat);
                        std::copy(temp.begin(), temp.end(), dest);
                    } else {
                        std::copy(buckets[idx].array.begin(), buckets[idx].array.end(), dest);
                    }
                }
            }
        }
        CALI_MARK_END("comm_large");
        CALI_MARK_END("comm");

        n = new_size;
    }

    return a;
}

bool check_correctness(const std::vector<unsigned int>& a, const std::vector<int>& p_n, int rank, int size) {
    CALI_MARK_BEGIN("correctness_check");
    bool is_sorted = validate_sort(a);
    
    #if DEBUG
    print_array(size, rank, a, p_n);
    #endif

    if (rank == 0) {
        if (is_sorted) {
            printf("Sorting successful\n");
        } else {
            printf("Sorting failed: Array is NOT sorted.\n");
        }
    }
    CALI_MARK_END("correctness_check");
    return is_sorted;
}

int main(int argc, char** argv) {
    CALI_CXX_MARK_FUNCTION;
    CALI_MARK_BEGIN("main");

    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 3) {
        if (rank == 0) usage("Incorrect number of arguments!");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    // Initialize Caliper and Adiak
    cali_init();
    cali::ConfigManager mgr;
    mgr.start();


    unsigned int global_seq_size = round((pow(2, std::stoi(argv[1]))));
    unsigned int flag = std::atoi(argv[2]);

    adiak::init(NULL);
    adiak::launchdate();
    adiak::libraries();
    adiak::cmdline();
    adiak::clustername();
    adiak::value("algorithm", "radix");
    adiak::value("programming_model", "mpi");
    adiak::value("data_type", "unsigned int");
    adiak::value("size_of_data_type", sizeof(unsigned int));
    adiak::value("input_size", global_seq_size);
    adiak::value("input_type", flag);
    adiak::value("num_procs", size);
    adiak::value("scalability", "strong");
    adiak::value("group_num", 25);
    adiak::value("implementation_source", "online");

    // Generate input data
    CALI_MARK_BEGIN("data_init_runtime");
    std::vector<unsigned int> a = decentralized_generation(rank, size, global_seq_size, flag);
    int n = a.size();
    std::vector<List> buckets(B);
    CALI_MARK_END("data_init_runtime");

    // Sort the data
    MPI_Barrier(MPI_COMM_WORLD);
    a = radix_sort(a, buckets, size, rank, n);
    MPI_Barrier(MPI_COMM_WORLD);

    // Exchange array sizes
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_small");
    std::vector<int> p_n(size);
    p_n[rank] = n;

    MPI_Request req;
    MPI_Status stat;

    for (int i = 0; i < size; i++) {
        if (i != rank) {
            MPI_Isend(&n, 1, MPI_INT, i, NUM_TAG, MPI_COMM_WORLD, &req);
        }
    }

    for (int i = 0; i < size; i++) {
        if (i != rank) {
            MPI_Recv(&p_n[i], 1, MPI_INT, i, NUM_TAG, MPI_COMM_WORLD, &stat);
        }
    }
    CALI_MARK_END("comm_small");
    CALI_MARK_END("comm");

    // Check correctness
    check_correctness(a, p_n, rank, size);

    CALI_MARK_END("main");
    
    mgr.stop();
    mgr.flush();
    MPI_Finalize();
    return 0;
}
