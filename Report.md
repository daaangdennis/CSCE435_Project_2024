# CSCE 435 Group project

## 0. Group number:

25

## 1. Group members:

1. Jack Couture
2. Deric Le
3. Jose Ortiz
4. Sam Zhang
5. Dennis Dang

We will communicate using Slack and everyone will be responsible for their algorithm and its requirements.

## 2. Project topic

Parallel Sorting Algorithms

### 2a. Brief project description (what algorithms will you be comparing and on what architectures)

For this project, we will be comparing the sorting algorithms listed below:

- Bitonic Sort (Dennis Dang): Bitonic sort is a sorting algorithm that first creates a bitonic sequence and then performs a bitonic merge to create a sorted sequence. In order to make a parallel program for bitonic sort, the array data will be split among $p$ processors. Each processor will then sort their data either ascending or descending based on rank number and then use MPI to create the bitonic sequence. Then, MPI will be used again to allow the processors to communicate and exchange data between one another to perform the bitonic merge.

- Samplesort (Sam Zhang): samplesort is a generalization of quick sort used in parallel processing systems. Samplesort partitions an unsorted list into $k$ buckets, and sorts each bucket. In parallel computing, $p$ buckets are assigned to $p$ processors, allowing efficient sorting of buckets.

- Merge Sort (Jack Couture): Merge sort is a sorting algorithm that divides up the data to work more efficiently and can be used to rapidly sort with parallel computing much larger sets of data. It will take the data and divide them among $p$ processors which will run in parallel to sort the divided segments. This allows the large number of buckets and data to be much more rapidly sorted.

- Radix Sort (Deric Le): A sorting algorithm that compares the digits/characters of each element instead of the entire element. Since each sorting step is independent of eachother, it can be parallelized efficiently. For each element, we can create a groups based on the index of each digit/character. For example, given a list [23,1,789], we will have three groups: [3,1,9], [2,None,8], [None,None,7], where each group corresponds to the digits at the same index across the elements. Next, we can sort each group independently, and then reconstruct them to output the sorted list.

- Column Sort (Jose Ortiz): Columnsort is a parallel sorting algorithm that arranges the input into an $r$ x $s$ matrix, where each processor manages a subset of columns. The algorithm operates through multiple rounds of sorting and permutations. First, the columns are independently sorted. Then, a fixed permutation is applied to redistribute elements across the columns. This process of column sorting and inter-column permutations repeats until the rows of the matrix are fully sorted. The first column will hold the smallest elements, sorted from top to bottom, followed by the next columns in order. To achieve parallel Columnsort using MPI, the matrix is distributed across multiple processors, with each processor responsible for managing and sorting a subset of the columns. MPI enables efficient communication between processors during the sorting and permutation phases.

We will use MPI for message passing and code in C++.

### 2b. Pseudocode for each parallel algorithm

- For MPI programs, include MPI calls you will use to coordinate between processes

  - Bitonic Sort (Dennis Dang):

    ```
    Initialize MPI

    If rank == MASTER:
      Generate data and split the data among the number of processors using MPI_Scatter

    if rank % 2 == 0
      Locally sort data into ascending order using any standard sorting algorithm
    else: // rank % 2 != 0
      Locally sort data into descending order using any standard sorting algorithm

    Create bitonic sequence using MPI_Send and MPI_Recv
    Perform bitonic merging between processors using MPI_Send and MPI_Recv
    Gather sorted data to the Master process using MPI_Gather

    Finalize MPI
    ```

  - Samplesort (Sam Zhang):

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
      # exchange buckets
      for i in {1,2,...,num_processors}:
        if (pid == i):
          # receive buckets corresponding to my pid from every processor
          for j in {1,2,...,num_processors}:
            if (j != pid):
              recv_bucket_size <- recv(1, ULONG, j)
              recv_bucket <- recv(recv_bucket_size, DType, j)
              recv_vals.append(flatten(recv_bucket))
            else:
              recv_vals.append(flatten(buckets[pid]))
        else:
          # send i-th bucket to the i-th processor
          send(len(bucket[i]), 1, ULONG, i)
          send(buckets[i], len(buckets[i]), DType, i)
      # sort received values
      sorted_recv_vals = sort(recv_vals)
    ```

  - Merge Sort (Jack Couture):

    ```
    //Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes); // Get number of processors
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get rank of the current process

    //Distribute input data
    if (rank == 0)
    {
      input_data = ReadInputData();
      total_elements = size_of(input_data); // total number of elements to sort
      elements_per_process = total_elements / num_processes;

    //Distribute segments of input data to each process
      for (i = 1; i < num_processes; i++)
      {
          start_index = i * elements_per_process;
          MPI_Send(&input_data[start_index], elements_per_process, MPI_INT, i, 0, MPI_COMM_WORLD);
      }

    //Master keeps the first portion of the data
      local_data = input_data[0:elements_per_process];
    }
    else
    {
    //Other processes receive their segments of the array
      MPI_Recv(&local_data, elements_per_process, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    //Each process sorts its local segment
    local_sorted_data = sort(local_data);

    //Gather sorted segments at the master process
    if (rank == 0)
    {
      sorted_data = InitializeArray(total_elements);
      copy(local_sorted_data, sorted_data[0:elements_per_process]);

    // Receive sorted segments from other processes
      for (i = 1; i < num_processes; i++)
      {
          start_index = i * elements_per_process;
          MPI_Recv(&sorted_data[start_index], elements_per_process, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }

    //Merge all sorted segments at the master process
      final_sorted_data = MergeSortedSegments(sorted_data, num_processes, elements_per_process);

    }
    else
    {
    // Other processes send their sorted segment to the master process
      MPI_Send(local_sorted_data, elements_per_process, MPI_INT, 0, 1, MPI_COMM_WORLD);
    }

    // Finalize MPI
    MPI_Finalize();
    ```

  - Radix Sort (Deric Le):

    ```
    MPI_Init()

    MPI_Comm_size()
    MPI_Comm_rank()

    // master process distributes data to all processes
    if rank == 0:
        elements = [23, 1, 789, ...]

        biggest_element = get_max(elements)

        // distribute the elements to each worker evenly
        for process in processes:
            MPI_Send(elements, worker)

    // worker process receive, calculate, send
    else:
        // worker processes receive the elements
        MPI_Recv(received_elements)

        // worker processes sort the groups
        for digit_index in range(max_digits):
            current_digits = extract_group(received_elements, digit_index)
            sorted_group = sort(current_digits)

            MPI_Send(sorted_group, master)

    // master process receives sorted groups
    if rank == 0:
        // receive the sorted groups from workers
        MPI_Recv(sorted_group, worker)

        result = combine_sorted_groups(sorted_group)
        print(result)

    MPI_Finalize()
    ```

  - Column Sort (Jose Ortiz):

    ```
    // initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // define matrix dimensions
    int r = ...;  // number of rows
    int s = ...;  // number of columns
    int local_columns = s / num_procs;  // num of columns per processor
    int local_matrix[r][local_columns];  // column handled by each processor

    // distribute data across processors
    if (rank == 0) {
        // send each processor its subset of columns
        for (int p = 1; p < num_procs; p++) {
            MPI_Send(&matrix[r][p * local_columns], r * local_columns, MPI_INT, p, 0, MPI_COMM_WORLD);
        }
    } else {
        // recieve matrix part on non-master processors
        MPI_Recv(&local_matrix, r * local_columns, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    for (int iter = 0; iter < num_iterations; iter++) {
        // sort columns locally
        for (int col = 0; col < local_columns; col++) {
            sort_column(local_matrix, r, col);
        }

        // perform fixed permutation
        if (rank == 0) {
            for (int p = 1; p < num_procs; p++) {
                // gather sorted array from each column
                MPI_Recv(&local_matrix, r * local_columns, MPI_INT, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }

            // logic for matrix permutation
            permute_matrix(local_matrix, r, s);

            // redistritube permuted matrix back to processors
            for (int p = 1; p < num_procs; p++) {
                MPI_Send(&matrix[r][p * local_columns], r * local_columns, MPI_INT, p, 0, MPI_COMM_WORLD);
            }
        } else {
            // sorted column back to rank 0 for perm
            MPI_Send(&local_matrix, r * local_columns, MPI_INT, 0, 0, MPI_COMM_WORLD);

            // permuted matrix back from 0
            MPI_Recv(&local_matrix, r * local_columns, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        // repeat untill matrix is fully sorted
    }

    // gather final sorted data from all processors
    if (rank == 0) {
        for (int p = 1; p < num_procs; p++) {
            MPI_Recv(&matrix[r][p * local_columns], r * local_columns, MPI_INT, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    } else {
        MPI_Send(&local_matrix, r * local_columns, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    // finalize MPI
    MPI_Finalize();
    ```

### 2c. Evaluation plan - what and how will you measure and compare

- Input sizes, Input types
  - For our inputs, we will be using arrays of integers of size 2<sup>16</sup>, 2<sup>18</sup>, 2<sup>20</sup>, 2<sup>22</sup>, 2<sup>24</sup>, 2<sup>26</sup>, and 2<sup>28</sup>
  - Our input types will include arrays containing random data, sorted data, reverse sorted data, and 1% perturbed
- MPI number of processes:
  - 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024
- Strong scaling (same problem size, increase number of processors/nodes)
- Weak scaling (increase problem size, increase number of processors)

### 3a. Caliper instrumentation

Please use the caliper build `/scratch/group/csce435-f24/Caliper/caliper/share/cmake/caliper`
(same as lab2 build.sh) to collect caliper files for each experiment you run.

Your Caliper annotations should result in the following calltree
(use `Thicket.tree()` to see the calltree):

```
main
|_ data_init_X      # X = runtime OR io
|_ comm
|    |_ comm_small
|    |_ comm_large
|_ comp
|    |_ comp_small
|    |_ comp_large
|_ correctness_check
```

Required region annotations:

- `main` - top-level main function.
  - `data_init_X` - the function where input data is generated or read in from file. Use _data_init_runtime_ if you are generating the data during the program, and _data_init_io_ if you are reading the data from a file.
  - `correctness_check` - function for checking the correctness of the algorithm output (e.g., checking if the resulting data is sorted).
  - `comm` - All communication-related functions in your algorithm should be nested under the `comm` region.
    - Inside the `comm` region, you should create regions to indicate how much data you are communicating (i.e., `comm_small` if you are sending or broadcasting a few values, `comm_large` if you are sending all of your local values).
    - Notice that auxillary functions like MPI_init are not under here.
  - `comp` - All computation functions within your algorithm should be nested under the `comp` region.
    - Inside the `comp` region, you should create regions to indicate how much data you are computing on (i.e., `comp_small` if you are sorting a few values like the splitters, `comp_large` if you are sorting values in the array).
    - Notice that auxillary functions like data_init are not under here.
  - `MPI_X` - You will also see MPI regions in the calltree if using the appropriate MPI profiling configuration (see **Builds/**). Examples shown below.

All functions will be called from `main` and most will be grouped under either `comm` or `comp` regions, representing communication and computation, respectively. You should be timing as many significant functions in your code as possible. **Do not** time print statements or other insignificant operations that may skew the performance measurements.

### **Nesting Code Regions Example** - all computation code regions should be nested in the "comp" parent code region as following:

```
CALI_MARK_BEGIN("comp");
CALI_MARK_BEGIN("comp_small");
sort_pivots(pivot_arr);
CALI_MARK_END("comp_small");
CALI_MARK_END("comp");

# Other non-computation code
...

CALI_MARK_BEGIN("comp");
CALI_MARK_BEGIN("comp_large");
sort_values(arr);
CALI_MARK_END("comp_large");
CALI_MARK_END("comp");
```

### **Calltrees**:

```
# Bitonic Sort
0.666 main
├─ 0.000 MPI_Init
├─ 0.088 main
│  ├─ 0.003 data_init_runtime
│  ├─ 0.012 comp
│  │  ├─ 0.010 comp_small
│  │  └─ 0.003 comp_large
│  ├─ 0.057 comm
│  │  ├─ 0.023 comm_small
│  │  │  └─ 0.023 MPI_Sendrecv
│  │  └─ 0.034 MPI_Barrier
│  └─ 0.014 correctness_check
│     ├─ 0.000 MPI_Recv
│     ├─ 0.000 MPI_Send
│     ├─ 0.013 MPI_Reduce
│     └─ 0.000 MPI_Bcast
├─ 0.000 MPI_Finalize
├─ 0.000 MPI_Initialized
├─ 0.000 MPI_Finalized
└─ 0.020 MPI_Comm_dup
```

```
# Samplesort
1.614 main
├─ 0.003 MPI_Comm_dup
├─ 0.000 MPI_Finalize
├─ 0.000 MPI_Finalized
├─ 0.000 MPI_Init
├─ 0.000 MPI_Initialized
├─ 0.002 correctness_check
│  ├─ 0.000 MPI_Bcast
│  ├─ 0.000 MPI_Recv
│  ├─ 0.000 MPI_Reduce
│  └─ 0.000 MPI_Send
├─ 0.002 data_init_runtime
└─ 0.087 sort_runtime
   ├─ 0.000 MPI_Barrier
   ├─ 0.061 comm
   │  ├─ 0.043 comm_large
   │  │  ├─ 0.002 MPI_Gather
   │  │  └─ 0.040 MPI_Gatherv
   │  └─ 0.019 comm_small
   │     ├─ 0.017 MPI_Bcast
   │     └─ 0.002 MPI_Gather
   └─ 0.026 comp
      ├─ 0.025 comp_large
      └─ 0.000 comp_small
```

```
# Merge Sort
0.624 main
├─ 0.000 MPI_Init
├─ 0.079 main
│  ├─ 0.003 data_init_runtime
│  ├─ 0.006 comp
│  │  ├─ 0.005 comp_small
│  │  └─ 0.002 comp_large
│  ├─ 0.049 comm
│  │  ├─ 0.049 comm_small
│  │  │  ├─ 0.008 MPI_Recv
│  │  │  ├─ 0.000 MPI_Send
│  │  │  ├─ 0.045 MPI_Barrier
│  │  │  └─ 0.000 MPI_Bcast
│  │  └─ 0.000 comm_large
│  │     ├─ 0.000 MPI_Recv
│  │     ├─ 0.000 MPI_Send
│  │     └─ 0.000 MPI_Scatterv
│  └─ 0.020 correctness_check
│     ├─ 0.003 MPI_Recv
│     ├─ 0.000 MPI_Send
│     ├─ 0.017 MPI_Reduce
│     └─ 0.000 MPI_Bcast
├─ 0.000 MPI_Finalize
├─ 0.000 MPI_Initialized
├─ 0.000 MPI_Finalized
└─ 0.020 MPI_Comm_dup
```

```
# Radix Sort
0.393 main
├─ 0.000 MPI_Init
├─ 0.026 main
│  ├─ 0.001 data_init_runtime
│  ├─ 0.024 comp_large
│  │  ├─ 0.006 MPI_Barrier
│  │  └─ 0.018 radix_sort
│  │     ├─ 0.003 comp_large
│  │     ├─ 0.015 comm_large
│  │     │  ├─ 0.002 MPI_Isend
│  │     │  └─ 0.010 MPI_Recv
│  │     └─ 0.000 comp_small
│  ├─ 0.000 comm_small
│  │  ├─ 0.000 MPI_Isend
│  │  └─ 0.000 MPI_Recv
│  └─ 0.000 comp_small
├─ 0.000 MPI_Finalize
├─ 0.000 MPI_Initialized
├─ 0.000 MPI_Finalized
└─ 0.000 MPI_Comm_dup
```

```
# Column Sort
0.386 main
├─ 0.000 MPI_Init
├─ 0.014 main
│  ├─ 0.001 data_init_runtime
│  └─ 0.013 correctness_check
│     ├─ 0.000 MPI_Recv
│     ├─ 0.000 MPI_Send
│     ├─ 0.012 MPI_Reduce
│     └─ 0.000 MPI_Bcast
├─ 0.000 MPI_Finalize
├─ 0.000 MPI_Initialized
├─ 0.000 MPI_Finalized
└─ 0.001 MPI_Comm_dup
```

### 3b. Collect Metadata

Have the following code in your programs to collect metadata:

```
adiak::init(NULL);
adiak::launchdate();    // launch date of the job
adiak::libraries();     // Libraries used
adiak::cmdline();       // Command line used to launch the job
adiak::clustername();   // Name of the cluster
adiak::value("algorithm", algorithm); // The name of the algorithm you are using (e.g., "merge", "bitonic")
adiak::value("programming_model", programming_model); // e.g. "mpi"
adiak::value("data_type", data_type); // The datatype of input elements (e.g., double, int, float)
adiak::value("size_of_data_type", size_of_data_type); // sizeof(datatype) of input elements in bytes (e.g., 1, 2, 4)
adiak::value("input_size", input_size); // The number of elements in input dataset (1000)
adiak::value("input_type", input_type); // For sorting, this would be choices: ("Sorted", "ReverseSorted", "Random", "1_perc_perturbed")
adiak::value("num_procs", num_procs); // The number of processors (MPI ranks)
adiak::value("scalability", scalability); // The scalability of your algorithm. choices: ("strong", "weak")
adiak::value("group_num", group_number); // The number of your group (integer, e.g., 1, 10)
adiak::value("implementation_source", implementation_source); // Where you got the source code of your algorithm. choices: ("online", "ai", "handwritten").
```

They will show up in the `Thicket.metadata` if the caliper file is read into Thicket.

### **See the `Builds/` directory to find the correct Caliper configurations to get the performance metrics.** They will show up in the `Thicket.dataframe` when the Caliper file is read into Thicket.

## 4. Performance evaluation

Include detailed analysis of computation performance, communication performance.
Include figures and explanation of your analysis.

### 4a. Vary the following parameters

For input_size's:

- 2^16, 2^18, 2^20, 2^22, 2^24, 2^26, 2^28

For input_type's:

- Sorted, Random, Reverse sorted, 1%perturbed

MPI: num_procs:

- 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024

This should result in 4x7x10=280 Caliper files for your MPI experiments.

<ins>Note:</ins> Due to issues with Grace, we did not do runs for 1024 processes, resulting in 4x7x9=252 Caliper files for our MPI experiments.

### 4b. Hints for performance analysis

To automate running a set of experiments, parameterize your program.

- input_type: "Sorted" could generate a sorted input to pass into your algorithms
- algorithm: You can have a switch statement that calls the different algorithms and sets the Adiak variables accordingly
- num_procs: How many MPI ranks you are using

When your program works with these parameters, you can write a shell script
that will run a for loop over the parameters above (e.g., on 64 processors,
perform runs that invoke algorithm2 for Sorted, ReverseSorted, and Random data).

### 4c. You should measure the following performance metrics

- `Time`
  - Min time/rank
  - Max time/rank
  - Avg time/rank
  - Total time
  - Variance time/rank

- ### <ins>Note:</ins> Due to issues with Grace, plot points for 1024 processors will not be shown.

- ### Bitonic Sort (Dennis Dang)
  - #### Strong Scaling:
    For all input types, the average time for the comp region decreased as more processors were used. This is expected as using more processors means that the data will be split into smaller chunks between processors, making the overall local sorting much quicker. Furthermore, this decrease was more prevalent in larger input sizes due to the fact that sorting smaller input sizes is already a fast computation. As for the main region, for every input type, the average time actually increased as more processors were used for input sizes 2<sup>16</sup>, 2<sup>18</sup>, and 2<sup>20</sup>. We suspect that this is because adding more processors for smaller input sizes just increases the overhead for communicating between processors with little benefit towards the actual computation time. This interaction can also kind of be seen with input sizes 2<sup>22</sup> and 2<sup>24</sup>, where the average time decreased as the number of processors increased, up to 16 processors, at which point, the time started to increase. With the larger input sizes of 2<sup>26</sup> and 2<sup>28</sup>, the average time constantly decreased with no increase at any point, most likely because the benefit from having more processors outweighed the increased overhead.
    - ##### Avg time/rank:
        ![Strong Scaling Input 2^28](Plots/Bitonicsort_Plots/strong_scaling_28.jpg)
        ![Strong Scaling Input 2^26](Plots/Bitonicsort_Plots/strong_scaling_26.jpg)
        ![Strong Scaling Input 2^24](Plots/Bitonicsort_Plots/strong_scaling_24.jpg)
        ![Strong Scaling Input 2^22](Plots/Bitonicsort_Plots/strong_scaling_22.jpg)
        ![Strong Scaling Input 2^20](Plots/Bitonicsort_Plots/strong_scaling_20.jpg)
        ![Strong Scaling Input 2^18](Plots/Bitonicsort_Plots/strong_scaling_18.jpg)
        ![Strong Scaling Input 2^16](Plots/Bitonicsort_Plots/strong_scaling_16.jpg)
  - #### Strong Scaling Speedup
    - ##### Main Region:
      ![Strong Scaling Speedup Main Region](Plots/Bitonicsort_Plots/strong_scaling_speedup_main.jpg)
    - ##### Comp Region:
      ![Strong Scaling Speedup Comp Region](Plots/Bitonicsort_Plots/strong_scaling_speedup_comp.jpg)
    - ##### Comm Region:
      ![Strong Scaling Speedup Comm Region](Plots/Bitonicsort_Plots/strong_scaling_speedup_comm.jpg)
  - #### Weak Scaling
    - ##### Main Region:
      ![Strong Scaling Speedup Main Region](Plots/Bitonicsort_Plots/weak_scaling_main.jpg)
    - ##### Comp Region:
      ![Strong Scaling Speedup Comp Region](Plots/Bitonicsort_Plots/weak_scaling_comp.jpg)
    - ##### Comm Region:
      ![Strong Scaling Speedup Comm Region](Plots/Bitonicsort_Plots/weak_scaling_comm.jpg)

- ### Samplesort (Sam Zhang)
  - #### Strong Scaling (Avg time/rank):
    The strong scaling analysis for samplesort suggests the runtime does not necessarily decreases as the number of processors increases. Even more interestingly, we observe using more processors can lead to slower runtime when the input size is small. This observation is caused by the communication overhead introduced with using more processors, and the plots produced using the "comm" region best illustrate this overhead. Although using more processors on smaller inputs can lead to longer runtime, using more processors on larger inputs does lead to shorter runtime, which is expected because using more processors lead to greater amount of shorter buckets, which means sorting them require less time. Another interesting note is that we observe the perturbed input require longer sorting time than other input types, and this is because of bad sample selections. Our samplesort algorithm always select the first, the last, and evenly spaced samples from local sequences, but our data generation algorithm always have the first element of the local sequence randomized when perturbed flag is selected. This means we can guarantee at least one bad sample from each local sequence, which can lead to worst-case splitter selection, causing buckets to have uneven sizes.
    - ##### Input Size 2^16:
      ![](Plots/Samplesort_Plots/samplesort_main_strong_16.png)
      ![](Plots/Samplesort_Plots/samplesort_comm_strong_16.png)
      ![](Plots/Samplesort_Plots/samplesort_comp_strong_16.png)
    - ##### Input Size 2^18:
      ![](Plots/Samplesort_Plots/samplesort_main_strong_18.png)
      ![](Plots/Samplesort_Plots/samplesort_comm_strong_18.png)
      ![](Plots/Samplesort_Plots/samplesort_comp_strong_18.png)
    - ##### Input Size 2^20:
      ![](Plots/Samplesort_Plots/samplesort_main_strong_20.png)
      ![](Plots/Samplesort_Plots/samplesort_comm_strong_20.png)
      ![](Plots/Samplesort_Plots/samplesort_comp_strong_20.png)
    - ##### Input Size 2^22:
      ![](Plots/Samplesort_Plots/samplesort_main_strong_22.png)
      ![](Plots/Samplesort_Plots/samplesort_comm_strong_22.png)
      ![](Plots/Samplesort_Plots/samplesort_comp_strong_22.png)
    - ##### Input Size 2^24:
      ![](Plots/Samplesort_Plots/samplesort_main_strong_24.png)
      ![](Plots/Samplesort_Plots/samplesort_comm_strong_24.png)
      ![](Plots/Samplesort_Plots/samplesort_comp_strong_24.png)
    - ##### Input Size 2^26:
      ![](Plots/Samplesort_Plots/samplesort_main_strong_26.png)
      ![](Plots/Samplesort_Plots/samplesort_comm_strong_26.png)
      ![](Plots/Samplesort_Plots/samplesort_comp_strong_26.png)
    - ##### Input Size 2^28:
      ![](Plots/Samplesort_Plots/samplesort_main_strong_28.png)
      ![](Plots/Samplesort_Plots/samplesort_comm_strong_28.png)
      ![](Plots/Samplesort_Plots/samplesort_comp_strong_28.png)
  - #### Strong Scaling Speedup (Avg time/rank):
    The strong scaling speedup analysis for samplesort further confirms the observation using more processors does not necessarily lead to efficiency increases. The plots produced using the "main" region shows the speedup for small input size is low, while those for large input size is significantly higher. The plots produced using the "comm" region further highlights the communication overhead caused by using more processors. As shown in the "comm" region graphs, the speedup approaches 0 when the input size is small.
    - ##### Sorted:
      ![](Plots/Samplesort_Plots/samplesort_main_speedup_sorted.png)
      ![](Plots/Samplesort_Plots/samplesort_comm_speedup_sorted.png)
      ![](Plots/Samplesort_Plots/samplesort_comp_speedup_sorted.png)
    - ##### Reverse:
      ![](Plots/Samplesort_Plots/samplesort_main_speedup_reverse.png)
      ![](Plots/Samplesort_Plots/samplesort_comm_speedup_reverse.png)
      ![](Plots/Samplesort_Plots/samplesort_comp_speedup_reverse.png)
    - ##### Perturbed:
      ![](Plots/Samplesort_Plots/samplesort_main_speedup_perturbed.png)
      ![](Plots/Samplesort_Plots/samplesort_comm_speedup_perturbed.png)
      ![](Plots/Samplesort_Plots/samplesort_comp_speedup_perturbed.png)
    - ##### Random:
      ![](Plots/Samplesort_Plots/samplesort_main_speedup_random.png)
      ![](Plots/Samplesort_Plots/samplesort_comm_speedup_random.png)
      ![](Plots/Samplesort_Plots/samplesort_comp_speedup_random.png)
  - #### Weak Scaling (Avg time/rank):
    - ##### Sorted:
      ![](Plots/Samplesort_Plots/samplesort_main_weak_sorted.png)
      ![](Plots/Samplesort_Plots/samplesort_comm_weak_sorted.png)
      ![](Plots/Samplesort_Plots/samplesort_comp_weak_sorted.png)
    - ##### Reverse:
      ![](Plots/Samplesort_Plots/samplesort_main_weak_reverse.png)
      ![](Plots/Samplesort_Plots/samplesort_comm_weak_reverse.png)
      ![](Plots/Samplesort_Plots/samplesort_comp_weak_reverse.png)
    - ##### Perturbed:
      ![](Plots/Samplesort_Plots/samplesort_main_weak_perturbed.png)
      ![](Plots/Samplesort_Plots/samplesort_comm_weak_perturbed.png)
      ![](Plots/Samplesort_Plots/samplesort_comp_weak_perturbed.png)
    - ##### Random:
      ![](Plots/Samplesort_Plots/samplesort_main_weak_random.png)
      ![](Plots/Samplesort_Plots/samplesort_comm_weak_random.png)
      ![](Plots/Samplesort_Plots/samplesort_comp_weak_random.png)
        
- ### Merge Sort ((Jack Couture))
  With the caliper files there were a multitude of issues regading the connection with Grace. However, a connection was established long enough to produce caliper files and ensuer that the local results were able to be scaled. Future scripts will be an to fulfill the needs of the report and presentation and graphed using Jupyter.

## 5. Presentation

Plots for the presentation should be as follows:

- For each implementation:
  - For each of comp_large, comm, and main:
    - Strong scaling plots for each input_size with lines for input_type (7 plots - 4 lines each)
    - Strong scaling speedup plot for each input_type (4 plots)
    - Weak scaling plots for each input_type (4 plots)

Analyze these plots and choose a subset to present and explain in your presentation.

## 6. Final Report

Submit a zip named `TeamX.zip` where `X` is your team number. The zip should contain the following files:

- Algorithms: Directory of source code of your algorithms.
- Data: All `.cali` files used to generate the plots seperated by algorithm/implementation.
- Jupyter notebook: The Jupyter notebook(s) used to generate the plots for the report.
- Report.md
