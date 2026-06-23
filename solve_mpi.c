#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "solve_mpi.h"
#include "cube.h"
#include "solve.h"

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


bool solveCubeWithMPIScatter(CubeState cube, int length) {
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
    }

    MPI_Bcast(&totalJobs, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int jobsPerRank = (int)(totalJobs / size); //Round result

    MPISearchJob *localJobs = malloc(sizeof(MPISearchJob) * jobsPerRank);

    MPI_Scatter(
        allJobs,
        jobsPerRank * sizeof(MPISearchJob),
        MPI_BYTE,
        localJobs,
        jobsPerRank * sizeof(MPISearchJob),
        MPI_BYTE,
        0,
        MPI_COMM_WORLD
    );

    printf("Rank %d received %d jobs\n", rank, jobsPerRank);

    bool localFound = false;
    
    for (int i = 0; i < jobsPerRank; ++i) {
        MPISearchJob job = localJobs[i];

        if(depthFirstSearch(job.cube, job.remainingDepth)) {
            localFound = true;
            break;
        }
    }

    int localFoundInt = localFound ? 1 : 0;
    int globalFoundInt = 0;

    MPI_Reduce(&localFoundInt, &globalFoundInt, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        if(globalFoundInt) {
            printf("Solution found\n");
        } else {
            printf("No solution found\n");
        }
    }

    free(localJobs);

    if (rank == 0) {
        free(allJobs);
    }

    MPI_Finalize();
    return true;
}

