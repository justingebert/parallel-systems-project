#ifndef SOLVE_MPI_H
#define SOLVE_MPI_H

#include "cube.h"
#include <stdbool.h>

typedef struct {
    CubeState cube;
    int remainingDepth;
} MPISearchJob;

bool solveCubeWithMPIScatter(CubeState cube, int length);
bool solveCubeWithMPIMasterWorker(CubeState cube, int length, bool cancellable);


#endif