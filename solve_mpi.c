#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "solve_mpi.h"
#include "cube.h"

#define JOB_DEPTH 2

static int generateMPIJobs(CubeState cube, int currentDepth, int remainingDepth, MPISearchJob *jobs) {
    if (currentDepth == 0) {
        jobs[0].cube = cube;
        jobs[0].remainingDepth = remainingDepth;

        return 1;
    }

    CubeExpansion expansion = expand(cube);
    int count = 0;

    for (int i = 0; i < CUBE_MOVE_COUNT; i++) {
        count += generateMPIJobs(expansion.states[i], currentDepth - 1, remainingDepth - 1, jobs + count);
    }

    return count;
}


bool initMpi(CubeState cube, int length) {
    MPI_Init(NULL, NULL);

    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPISearchJob *allJobs = NULL;
    int totalJobs = 0;

    if (rank == 0) {
        int maxJobs = 1;
        for (int i = 0; i < JOB_DEPTH; i++) {
            maxJobs *= CUBE_MOVE_COUNT;
        }

        allJobs = malloc(sizeof(*allJobs) * (size_t)maxJobs);

        totalJobs = generateMPIJobs(cube, JOB_DEPTH, length, allJobs);

        printf("Generated %d jobs across %d MPI ranks\n", totalJobs, size);

        free(allJobs);
    } else {
        printf("Hello World from process %d of %d\n", rank, size);
    }


    MPI_Finalize();
    return true;
}

