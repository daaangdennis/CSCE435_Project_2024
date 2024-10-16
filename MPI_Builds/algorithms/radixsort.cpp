#include "radixsort.h"

unsigned bits(unsigned x, int k, int j)
{
    return (x >> k) & ~(~0u << j);
}

std::vector<int> radix_sort(std::vector<int> &a, std::vector<Bucket> &buckets, const int P, const int rank, int &n)
{
    std::vector<std::vector<int>> count(B, std::vector<int>(P));
    std::vector<int> l_count(B);
    int l_B = B / P;
    std::vector<std::vector<int>> p_sum(l_B, std::vector<int>(P));

    MPI_Request req;
    MPI_Status stat;

    for (int pass = 0; pass < N; pass++)
    {
        // init counts arrays
        for (int j = 0; j < B; j++)
        {
            count[j][rank] = 0;
            l_count[j] = 0;
            buckets[j].array.clear();
        }

        // count items per bucket
        for (int i = 0; i < n; i++)
        {
            unsigned int idx = bits(a[i], pass * g, g);
            count[idx][rank]++;
            l_count[idx]++;
            buckets[idx].add_item(a[i]);
        }

        // do one-to-all transpose
        for (int p = 0; p < P; p++)
        {
            if (p != rank)
            {
                MPI_Isend(l_count.data(), B, MPI_INT, p, COUNTS_TAG_NUM, MPI_COMM_WORLD, &req);
            }
        }

        // receive counts from others
        for (int p = 0; p < P; p++)
        {
            if (p != rank)
            {
                MPI_Recv(l_count.data(), B, MPI_INT, p, COUNTS_TAG_NUM, MPI_COMM_WORLD, &stat);

                // populate counts per bucket for other processes
                for (int i = 0; i < B; i++)
                {
                    count[i][p] = l_count[i];
                }
            }
        }

        // calculate new size based on values received from all processes
        int new_size = 0;
        for (int j = 0; j < l_B; j++)
        {
            int idx = j + rank * l_B;
            for (int p = 0; p < P; p++)
            {
                p_sum[j][p] = new_size;
                new_size += count[idx][p];
            }
        }

        // reallocate array if newly calculated size is larger
        if (new_size > n)
        {
            a.resize(new_size);
        }

        // send keys of this process to others
        for (int j = 0; j < B; j++)
        {
            int p = j / l_B;   // determine which process this bucket belongs to
            int p_j = j % l_B; // transpose to that process local bucket index
            if (p != rank && !buckets[j].array.empty())
            {
                MPI_Isend(buckets[j].array.data(), buckets[j].array.size(), MPI_INT, p, p_j, MPI_COMM_WORLD, &req);
            }
        }

        // receive keys from other processes
        for (int j = 0; j < l_B; j++)
        {
            int idx = j + rank * l_B;
            for (int p = 0; p < P; p++)
            {
                int b_count = count[idx][p];
                if (b_count > 0)
                {
                    int *dest = &a[p_sum[j][p]];
                    if (rank != p)
                    {
                        MPI_Recv(dest, b_count, MPI_INT, p, j, MPI_COMM_WORLD, &stat);
                    }
                    else
                    {
                        std::copy(buckets[idx].array.begin(), buckets[idx].array.end(), dest);
                    }
                }
            }
        }

        // update new size
        n = new_size;
    }

    return a;
}