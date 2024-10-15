#ifndef RADIXSORT_H
#define RADIXSORT_H

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <cmath>
#include <algorithm>

#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

#include <vector>

// globals
constexpr int b = 32;          // number of bits for integer
constexpr int g = 8;           // group of bits for each scan
constexpr int N = b / g;       // number of passes
constexpr int B = 1 << g;      // number of buckets, 2^g

// MPI tags constants, offset by max bucket to avoid collisions
constexpr int COUNTS_TAG_NUM = B + 1;
constexpr int PRINT_TAG_NUM = COUNTS_TAG_NUM + 1;
constexpr int NUM_TAG = PRINT_TAG_NUM + 1;

class Bucket {
public:
    std::vector<int> array;

    void add_item(int item) {
        array.push_back(item);
    }
};

unsigned bits(unsigned x, int k, int j);
std::vector<int> radix_sort(std::vector<int>& a, std::vector<Bucket>& buckets, const int P, const int rank, int& n);

#endif

