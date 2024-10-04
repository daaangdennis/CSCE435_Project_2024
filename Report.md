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

- Sample Sort (Sam Zhang): sample sort is a generalization of quick sort used in parallel processing systems. Sample sort partitions an unsorted list into $k$ buckets, and sorts each bucket. In parallel computing, $p$ buckets are assigned to $p$ processors, allowing efficient sorting of buckets.

- Merge Sort (Jack Couture): Merge sort is a sorting algorithm that divides up the data to work more efficiently and can be used to rapidly sort with parallel computing much larger sets of data. It will take the data and divide them among $p$ processors which will run in parallel to sort the divided segments. This allows the large number of buckets and data to be much more rapidly sorted.

- Radix Sort (Deric Le): A sorting algorithm that compares the digits/characters of each element instead of the entire element. Since each sorting step is independent of eachother, it can be parallelized efficiently. For each element, we can create a groups based on the index of each digit/character. For example, given a list [23,1,789], we will have three groups: [3,1,9], [2,None,8], [None,None,7], where each group corresponds to the digits at the same index across the elements. Next, we can sort each group independently, and then reconstruct them to output the sorted list.  

- Column Sort (Jose Ortiz): Column sort is a parallel sorting algorithim that organizes data based on their column indices. To implement a parallel version of column sort using MPI, the algorithim divides the input into segments corresponding to different column indices and distributes these segments across multiple processors. Each processor will sort its segment independently using a standard sort algorithim. Once the columns are sorted, the next step is to merge them into a single output array while maintaining the sorted order. This process begins by initializing pointers for each sorted column to track the current smallest unmerged element. The algorithm then compares the current elements pointed to by these pointers and selects the smallest one to add to the final output array. After an element is added, the corresponding pointer is advanced to the next element in that column. This comparison and selection continue until all elements from all columns are merged into the output array.

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
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes); // num of processor
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // rank of process

    // distribute input data
    if (rank == 0) {
        // divide into segments for each column
        input_data = ReadInputData();
        num_columns = tasks_per_processors;

        // distribute columns to each process
        for (i = 0; i < num_processes; i++) {
            //send column to corresponding process
            MPI_Send(&input_data[column_start_index[i]], column_size, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    } else {
        // other processes receive their segments
        MPI_Recv(&local_data, column_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // sort segments
    local_sorted_arr = sort(local_data);

    // merge sorted columns
    if (rank == 0) {
        // initialize pointers sorted columns
        Init_pointers(num_columns, pointers);
        merged_output = Init_output_arr(total_elements);

        for (i = 0; i < total_elements; i++) {
            // compare current elements pointed
            min_index = Find_min_ind(pointers, local_sorted_arr);

            // find smallest elem to append to final arr
            merged_output[i] = local_sorted_arr[min_index];

            // advance pointer of column that was just used
            move_pointer(pointers, min_index);
        }

        // send merge output
        for (i = 1; i < num_processes; i++) {
            MPI_Send(merged_output, total_elements, MPI_INT, i, 1, MPI_COMM_WORLD);
        }
    } else {
        // processes receive merge output
        MPI_Recv(merged_output, total_elements, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // finalize MPI
    MPI_Finalize();
    ```

### 2c. Evaluation plan - what and how will you measure and compare

- Input sizes, Input types
  - For our inputs, we will be using arrays of integers of size 2<sup>14</sup>, 2<sup>20</sup>, and 2<sup>26</sup>
  - Our input types will include arrays containing random data, sorted data, reverse sorted data, and sorted data except for 1%
- Strong scaling (same problem size, increase number of processors/nodes) 
- Weak scaling (increase problem size, increase number of processors)
