# CSCE 435 Group project

## 0. Group number: 
25

## 1. Group members:
1. Jack Couture
2. Deric Le
3. Jose Ortiz
4. Sam Zhang
5. Dennis Dang

Will communicate using Slack and everyone will be responsible for their algorithm and its requirements

## 2. Project topic
Parallel Sorting Algorithms

### 2a. Brief project description (what algorithms will you be comparing and on what architectures)

For this project, we will be comparing the sorting algorithms listed below:

- Bitonic Sort (Dennis Dang):
- Sample Sort (Sam Zhang): sample sort is a generalization of quick sort used in parallel processing systems. Sample sort partitions an unsorted list into $k$ buckets, and sorts each bucket. In parallel computing, $p$ buckets are assigned to $p$ processors, allowing efficient sorting of buckets.
- Merge Sort (Jack Couture): 
- Radix Sort (Deric Le):
- Column Sort (Jose Ortiz):

We will use MPI for message passing and code in C++.

### 2b. Pseudocode for each parallel algorithm
- For MPI programs, include MPI calls you will use to coordinate between processes

  - Bitonic Sort (Dennis Dang):
  - Sample Sort (Sam Zhang):
    ```
    # Assumptions:
    # - Each processor has an unsorted sub-list.
    # - Goal is to ensure the i-th processor has a sorted sub-list satisfying,
    #   the maximum element held the by i-th processor is smaller than the 
    #   minimum element held by the (i + 1)-th processor.
    
    # Definitions:
    # - pid: processor id (Comm_rank).
    # - num_processors: total number of processors (Comm_size).
    # - L: the unsorted sub-list held on this processor.
    # - DType: data type of elements in L.
    # - k: number of samples to pick from L.
    # - sample(L,k): samples k random elements from L.
    # - sort(L): sequential quick sort, returns a sorted list.

    def sample_sort(pid: UINT, num_processors: UINT, L: LIST, DType: TYPE, k: UINT):
      # take k samples from the list I have and gather samples from others
      frag_p <- sample(L, k)
      samples <- All_Gather(frag_p, k, DType, k * num_processors)
      # sort gathered samples and choose pivots
      sorted_samples <- sort(samples)
      pivots <- []
      for i in {1, 2, ..., len(sorted_samples) - 1}:
        if ((i % floor(len(sorted_samples) / num_processors)) == 0):
          pivots.append(sorted_samples[i])
      # initialize buckets
      buckets <-[]
      for i in {1,2,...,num_processors}:
        buckets.append([])
      # put elements I have into buckets
      for element in L:
        for i in {1,2,...,num_processors}:
          if (element <= pivots[i]):
            buckets[i].append(element)
            break
      # sort each bucket
      for i in {1,2,...,num_processors}:
        buckets[i] <- sort(buckets[i])
      # send i-th bucket to the i-th processor
      recv_vals <- []
      for i in {1,2,...,num_processors}:
        send(len(bucket[i]), 1, ULONG, i)
        send(buckets[i], len(buckets[i]), DType, i) 
      # receive buckets corresponding to my pid from every processor
      for i in {1,2,...,num_processors}:
        if (i != pid):
          recv_bucket_size <- recv(1, ULONG, i)
          recv_bucket <- recv(recv_bucket_size, DType, i)
          recv_vals.append(flatten(recv_bucket))
        else:
          recv_vals.append(flatten(buckets[pid]))
      # sort received elements
      sorted_recv_vals = sort(recv_vals)
    ```
  - Merge Sort (Jack Couture): 
  - Radix Sort (Deric Le):
  - Column Sort (Jose Ortiz):

### 2c. Evaluation plan - what and how will you measure and compare
- Input sizes, Input types
  - For our input sizes, 10<sup>10</sup>, 10<sup>20</sup>, 10<sup>40</sup>, ...
  - Out input types will include random data, sorted data, reverse sorted data, and sorted data except for 1%
- Strong scaling (same problem size, increase number of processors/nodes) 
- Weak scaling (increase problem size, increase number of processors)
