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

// Radix sort elements while communicating between other MPI processes
std::vector<unsigned int> radix_sort(std::vector<unsigned int>& a, std::vector<List>& buckets, const int P, const int rank, int& n) {
    CALI_CXX_MARK_FUNCTION;
    
    std::vector<std::vector<int>> count(B, std::vector<int>(P));
    std::vector<int> l_count(B);
    int l_B = B / P;
    std::vector<std::vector<int>> p_sum(l_B, std::vector<int>(P));

    MPI_Request req;
    MPI_Status stat;

    for (int pass = 0; pass < N; pass++) {
        CALI_MARK_BEGIN("comp_large");
        // init counts arrays
        for (int j = 0; j < B; j++) {
            count[j][rank] = 0;
            l_count[j] = 0;
            buckets[j].array.clear();
        } 

        // count items per bucket
        for (int i = 0; i < n; i++) {
            unsigned int idx = bits(a[i], pass*g, g);
            count[idx][rank]++; 
            l_count[idx]++;
            buckets[idx].array.push_back(a[i]);
        }
        CALI_MARK_END("comp_large");

        CALI_MARK_BEGIN("comm_large");
        // do one-to-all transpose
        for (int p = 0; p < P; p++) {
            if (p != rank) {
                MPI_Isend(l_count.data(), B, MPI_INT, p, COUNTS_TAG_NUM, MPI_COMM_WORLD, &req);
            }
        }

        // receive counts from others
        for (int p = 0; p < P; p++) {
            if (p != rank) {
                MPI_Recv(l_count.data(), B, MPI_INT, p, COUNTS_TAG_NUM, MPI_COMM_WORLD, &stat);
                for (int i = 0; i < B; i++) {
                    count[i][p] = l_count[i];
                }
            }
        }
        CALI_MARK_END("comm_large");

        CALI_MARK_BEGIN("comp_small");
        // calculate new size based on values received from all processes
        int new_size = 0;
        for (int j = 0; j < l_B; j++) {
            int idx = j + rank * l_B;
            for (int p = 0; p < P; p++) {
                p_sum[j][p] = new_size;
                new_size += count[idx][p];
            }
        }

        // resize array if newly calculated size is larger
        if (new_size > n) {
            a.resize(new_size);
        }
        CALI_MARK_END("comp_small");

        CALI_MARK_BEGIN("comm_large");
        // send keys of this process to others
        for (int j = 0; j < B; j++) {
            int p = j / l_B;
            int p_j = j % l_B;
            if (p != rank && !buckets[j].array.empty()) {
                MPI_Isend(buckets[j].array.data(), buckets[j].array.size(), MPI_UNSIGNED, p, p_j, MPI_COMM_WORLD, &req);
            }
        }

        // receive keys from other processes
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

        // update new size
        n = new_size;
    }

    return a;
}

int main(int argc, char** argv)
{
    CALI_CXX_MARK_FUNCTION;

    int rank, size; 
    if (argc != 3) {
        if (rank == 0) usage("Incorrect number of arguments!");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    // parse args
    unsigned int global_seq_size = round((pow(2, std::stoi(argv[1]))));
    unsigned int flag = std::atoi(argv[2]);

    /*=============================================================================*/
    /*================================= init ======================================*/
    /*=============================================================================*/
    // init mpi
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // init cali
    cali_init();
    cali::ConfigManager mgr;
    mgr.start();

    // init adiak
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

    CALI_MARK_BEGIN("main");
    /*=============================================================================*/
    /*================================= generate ======================================*/
    /*=============================================================================*/
    CALI_MARK_BEGIN("data_init_runtime");
    std::vector<unsigned int> a = decentralized_generation(rank, size, global_seq_size, flag);
    int n = a.size();
    std::vector<List> buckets(B);
    CALI_MARK_END("data_init_runtime");

    #if DEBUG
    printf("init data from rank %d:\n", rank);
    for (const auto& val : a) {
        printf("%u\n", val);
    }
    #endif

    /*=============================================================================*/
    /*================================= sorting ======================================*/
    /*=============================================================================*/
    CALI_MARK_BEGIN("comp_large");
    MPI_Barrier(MPI_COMM_WORLD);
    a = radix_sort(a, buckets, size, rank, n);
    MPI_Barrier(MPI_COMM_WORLD);
    CALI_MARK_END("comp_large");

    /*=============================================================================*/
    /*================================= finalize ======================================*/
    /*=============================================================================*/
    CALI_MARK_BEGIN("comm_small");
    // each rank sets its index to its local array size
    std::vector<int> p_n(size);
    p_n[rank] = n;

    MPI_Request req;
    MPI_Status stat;

    // each rank sends local array size to all other ranks
    for (int i = 0; i < size; i++) {
        if (i != rank) {
            MPI_Isend(&n, 1, MPI_INT, i, NUM_TAG, MPI_COMM_WORLD, &req);
        }
    }

    // each rank receives array size of all other ranks into p_n
    for (int i = 0; i < size; i++) {
        if (i != rank) {
            MPI_Recv(&p_n[i], 1, MPI_INT, i, NUM_TAG, MPI_COMM_WORLD, &stat);
        }
    }
    CALI_MARK_END("comm_small");
    
    // print result
    CALI_MARK_BEGIN("comp_small");
    #if DEBUG
    print_array(size, rank, a, p_n);
    #endif

    // validate it
    bool is_sorted = validate_sort(a);
    if (rank == 0) {
        if (is_sorted) {
            printf("Sorting successful");
        } else {
            printf("Sorting failed: Array is NOT sorted.");
        }
    }
    CALI_MARK_END("comp_small");
    CALI_MARK_END("main");

    mgr.stop(); mgr.flush();
    MPI_Finalize();
    return 0;
}
