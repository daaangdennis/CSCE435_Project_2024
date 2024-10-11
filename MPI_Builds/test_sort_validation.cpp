#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <vector>
#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

#include "src/sort_validation.h"

int main (int argc, char *argv[]) {
    int pid, num_processors;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&pid);
    MPI_Comm_size(MPI_COMM_WORLD,&num_processors);
    unsigned int local_seq_size = 5;
    
    if (num_processors >= 2){
        /* sorted test case */
        std::vector<unsigned int> myvector;
        for (unsigned int i = 0; i < local_seq_size; ++i){
            myvector.push_back(local_seq_size * pid + i);
        }
        for (unsigned int i = 0; i < num_processors; ++i){
            MPI_Barrier(MPI_COMM_WORLD);
            if (pid == i){
                printf ("DEBUG [p%d]: seq = {", pid);
                for (unsigned int j = 0; j < local_seq_size; ++j){
                    printf("%d ", myvector.at(j));
                }
                printf("}.\n");
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
        bool is_everyone_sorted = sort_validation(myvector, pid, num_processors, MPI_COMM_WORLD);
        for (unsigned int i = 0; i < num_processors; ++i){
            MPI_Barrier(MPI_COMM_WORLD);
            if (i == pid){
                printf("DEBUG [p%d]: is_everyone_sorted = %d.\n", pid, is_everyone_sorted);
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }

        /* unsorted test case 1: unsorted local but sorted global*/
        myvector.clear();
        for (unsigned int i = 0; i < local_seq_size; ++i){
            myvector.push_back(local_seq_size * pid + i);
        }
        unsigned int temp = myvector.at(1);
        myvector.at(1) = myvector.at(myvector.size() - 2);
        myvector.at(myvector.size() - 2) = temp;
        for (unsigned int i = 0; i < num_processors; ++i){
            MPI_Barrier(MPI_COMM_WORLD);
            if (pid == i){
                printf ("DEBUG [p%d]: seq = {", pid);
                for (unsigned int j = 0; j < local_seq_size; ++j){
                    printf("%d ", myvector.at(j));
                }
                printf("}.\n");
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
        is_everyone_sorted = sort_validation(myvector, pid, num_processors, MPI_COMM_WORLD);
        for (unsigned int i = 0; i < num_processors; ++i){
            MPI_Barrier(MPI_COMM_WORLD);
            if (i == pid){
                printf ("DEBUG [p%d]: is_everyone_sorted = %d.\n", pid, is_everyone_sorted);
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
        temp = 0;

        /* unsorted test case 2: sorted local but unsorted global*/
        myvector.clear();
        for (unsigned int i = 0; i < (local_seq_size * num_processors); ++i){
            if ((i % num_processors) == pid){
                myvector.push_back(i);
            } 
        }
        for (unsigned int i = 0; i < num_processors; ++i){
            MPI_Barrier(MPI_COMM_WORLD);
            if (pid == i){
                printf ("DEBUG [p%d]: seq = {", pid);
                for (unsigned int j = 0; j < local_seq_size; ++j){
                    printf("%d ", myvector.at(j));
                }
                printf("}.\n");
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
        is_everyone_sorted = sort_validation(myvector, pid, num_processors, MPI_COMM_WORLD);
        for (unsigned int i = 0; i < num_processors; ++i){
            MPI_Barrier(MPI_COMM_WORLD);
            if (i == pid){
                printf ("DEBUG [p%d]: is_everyone_sorted = %d.\n", pid, is_everyone_sorted);
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }

        /* unsorted test case 3: unsorted local and unsorted global*/
        myvector.clear();
        for (unsigned int i = 0; i < (local_seq_size * num_processors); ++i){
            if ((i % num_processors) == pid){
                myvector.push_back(i);
            } 
        }
        temp = myvector.at(1);
        myvector.at(1) = myvector.at(myvector.size() - 2);
        myvector.at(myvector.size() - 2) = temp;
        for (unsigned int i = 0; i < num_processors; ++i){
            MPI_Barrier(MPI_COMM_WORLD);
            if (pid == i){
                printf ("DEBUG [p%d]: seq = {", pid);
                for (unsigned int j = 0; j < local_seq_size; ++j){
                    printf("%d ", myvector.at(j));
                }
                printf("}.\n");
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
        is_everyone_sorted = sort_validation(myvector, pid, num_processors, MPI_COMM_WORLD);
        for (unsigned int i = 0; i < num_processors; ++i){
            MPI_Barrier(MPI_COMM_WORLD);
            if (i == pid){
                printf ("DEBUG [p%d]: is_everyone_sorted = %d.\n", pid, is_everyone_sorted);
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
    }
    
    MPI_Finalize();
    return 0;
}
