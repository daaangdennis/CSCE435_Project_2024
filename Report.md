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

- Bitonic Sort (Dennis Dang):
- Sample Sort (Sam Zhang):
- Merge Sort (Jack Couture): 
- Radix Sort (Deric Le):
- Column Sort (Jose Ortiz):

### 2b. Pseudocode for each parallel algorithm
- For MPI programs, include MPI calls you will use to coordinate between processes

### 2c. Evaluation plan - what and how will you measure and compare
- Input sizes, Input types
  - For our input sizes, 10^10, 10^20, 10^40, ...
  - Out input types will include random data, sorted data, reverse sorted data, and sorted data except for 1%
- Strong scaling (same problem size, increase number of processors/nodes) 
- Weak scaling (increase problem size, increase number of processors)
