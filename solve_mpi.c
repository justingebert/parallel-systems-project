#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "solve_mpi.h"
#include "cube.h"
#include "solve.h"

#define JOB_DEPTH 2

#define TAG_REQUEST 1
#define TAG_WORK 2
#define TAG_FOUND 3
#define TAG_STOP 4

static int generateMPIJobs(CubeState cube, int currentDepth, int remainingDepth, MPISearchJob *jobs) {
    if (isSolved(cube)) {
        jobs[0].cube = cube;
        jobs[0].remainingDepth = 0;

        return 1;
    }     

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

static bool mpiStopRequested(void) {
    int stopWaiting = 0;

    MPI_Iprobe(0, TAG_STOP, MPI_COMM_WORLD, &stopWaiting, MPI_STATUS_IGNORE);

    if (stopWaiting == 0) {
        return false;
    }

    MPI_Recv(NULL, 0, MPI_BYTE, 0, TAG_STOP, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    return true;
}

static bool depthFirstSearchMPICancellable(CubeState cube, int length, int *nodesSinceCheck) {
    // Reduce overhead, only check after x nodes
    (*nodesSinceCheck)++;

    if (*nodesSinceCheck >= 1000) {
        *nodesSinceCheck = 0;

        if (mpiStopRequested()) {
            return false;
        }
    }

    if (isSolved(cube)) {
        return true;
    }

    if (length == 0) {
        return false;
    }

    CubeExpansion expansion = expand(cube);

    for (int i = 0; i < CUBE_MOVE_COUNT; i++) {
        if (depthFirstSearchMPICancellable(expansion.states[i], length - 1, nodesSinceCheck)) {
            return true;
        }
    }

    return false;
}


bool solveCubeWithMPIScatter(CubeState cube, int length) {
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

    MPI_Allreduce(&localFoundInt, &globalFoundInt, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

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

    return globalFoundInt != 0;
}

static bool runMPICoordinator(CubeState cube, int length, int size, bool cancellable) {
    MPISearchJob *allJobs = NULL;
    int totalJobs = 0;
    bool solutionFound = false;

    int maxJobs = 1;
    for (int i = 0; i < JOB_DEPTH; i++) {
        maxJobs *= CUBE_MOVE_COUNT;
    }

    allJobs = malloc(sizeof(*allJobs) * (size_t)maxJobs);

    totalJobs = generateMPIJobs(cube, JOB_DEPTH, length, allJobs);

    printf("Generated %d jobs for %d MPI ranks\n", totalJobs, size);
    
    const char *tagsMapping[] = {"TAG_REQUEST", "TAG_WORK", "TAG_FOUND", "TAG_STOP"};
    int nextJob = 0;
    int activeWorkers = size - 1;

    while (activeWorkers > 0) {
        MPI_Status status;

        MPI_Recv(
            NULL,
            0,
            MPI_BYTE,
            MPI_ANY_SOURCE,
            MPI_ANY_TAG,
            MPI_COMM_WORLD,
            &status
        );

        int workerId = status.MPI_SOURCE;
        int messageTag = status.MPI_TAG;

        printf("DEBUG (%d jobs left): Worker %d send a %s-message\n", maxJobs - nextJob, workerId, tagsMapping[messageTag-1]);

        if (messageTag == TAG_FOUND) {
            solutionFound = true;
            
            if (cancellable) {
                for (int worker = 1; worker < size; ++worker) {
                    if (worker != status.MPI_SOURCE) {
                        MPI_Send(
                            NULL,
                            0,
                            MPI_BYTE,
                            worker,
                            TAG_STOP,
                            MPI_COMM_WORLD
                        );
                    }  
                }

                activeWorkers = 0;
                break;
            }

            activeWorkers--;
            continue;
        }

        if (messageTag == TAG_REQUEST) {
            if (solutionFound || nextJob >= totalJobs) {
                MPI_Send(
                    NULL,
                    0,
                    MPI_BYTE,
                    workerId,
                    TAG_STOP,
                    MPI_COMM_WORLD
                );

                activeWorkers--;
                continue;
            } else {
                MPI_Send(
                    &allJobs[nextJob],
                    sizeof(MPISearchJob),
                    MPI_BYTE,
                    workerId,
                    TAG_WORK,
                    MPI_COMM_WORLD
                );

                nextJob++;
            }
        }
    }

    free(allJobs);

    if (solutionFound) {
        printf("Solution found\n");
    } else {
        printf("No solution found\n");
    }

    return solutionFound;
}

static bool runMPIWorker(bool cancellable) {
    bool localFound = false;

    while (true) {
        MPI_Send(
            NULL,
            0,
            MPI_BYTE,
            0,
            TAG_REQUEST,
            MPI_COMM_WORLD
        );

        MPISearchJob job;
        MPI_Status status;

        MPI_Recv(
            &job,
            sizeof(job),
            MPI_BYTE,
            0,
            MPI_ANY_TAG,
            MPI_COMM_WORLD,
            &status
        );

        int messageTag = status.MPI_TAG;

        if (messageTag == TAG_STOP) {
            break;
        }

        if (messageTag != TAG_WORK) {
            break;
        }

        if (cancellable) {
            int nodesSinceCheck = 0;

            localFound = depthFirstSearchMPICancellable(job.cube, job.remainingDepth, &nodesSinceCheck);
        } else {
            localFound = depthFirstSearch(job.cube, job.remainingDepth);
        }

        if(localFound) {
            MPI_Send(NULL, 0, MPI_BYTE, 0, TAG_FOUND, MPI_COMM_WORLD);

            break;
        }
    }

    return localFound;
}

bool solveCubeWithMPIMasterWorker(CubeState cube, int length, bool cancellable) {
    int rank;
    int size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        return runMPICoordinator(cube, length, size, cancellable);
    }

    return runMPIWorker(cancellable);
}

