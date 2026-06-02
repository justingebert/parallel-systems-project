#include <stdio.h>
#include <mpi.h>
#include <omp.h>

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    char proc_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(proc_name, &name_len);

    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        printf("MPI rank %d/%d | OpenMP thread %d | host %s\n",
               rank, size, thread_id, proc_name);
    }

    MPI_Finalize();
    return 0;
}
