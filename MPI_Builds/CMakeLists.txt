cmake_minimum_required(VERSION 3.12)

find_package(MPI REQUIRED)
find_package(caliper REQUIRED)
find_package(adiak REQUIRED)

# main executable
add_executable(main
    algorithms/bitonicsort.cpp
    algorithms/columnsort.cpp
    algorithms/decentralized_generation.cpp
    algorithms/mergesort.cpp
    algorithms/radixsort.cpp
    algorithms/samplesort.cpp
    algorithms/sort_validation.cpp
    main.cpp
)

# executable to test generation script
add_executable(test_decentralized_generation
    algorithms/bitonicsort.cpp
    algorithms/columnsort.cpp
    algorithms/decentralized_generation.cpp
    algorithms/mergesort.cpp
    algorithms/radixsort.cpp
    algorithms/samplesort.cpp
    algorithms/sort_validation.cpp
    tests/test_decentralized_generation.cpp
)

# executable to test validation script
add_executable(test_sort_validation
    algorithms/bitonicsort.cpp
    algorithms/columnsort.cpp
    algorithms/decentralized_generation.cpp
    algorithms/mergesort.cpp
    algorithms/radixsort.cpp
    algorithms/samplesort.cpp
    algorithms/sort_validation.cpp
    tests/test_sort_validation.cpp
)

message(STATUS "MPI includes : ${MPI_INCLUDE_PATH}")
message(STATUS "Caliper includes : ${caliper_INCLUDE_DIR}")
message(STATUS "Adiak includes : ${adiak_INCLUDE_DIRS}")
include_directories(SYSTEM ${MPI_INCLUDE_PATH})
include_directories(${caliper_INCLUDE_DIR})
include_directories(${adiak_INCLUDE_DIRS})

target_link_libraries(main PRIVATE MPI::MPI_CXX caliper)
target_link_libraries(test_decentralized_generation PRIVATE MPI::MPI_CXX caliper)
target_link_libraries(test_sort_validation PRIVATE MPI::MPI_CXX caliper)

