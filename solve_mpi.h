#ifndef SOLVE_MPI_H
#define SOLVE_MPI_H

#include "cube.h"
#include <stdbool.h>

typedef struct {
    CubeState cube;
    int remainingDepth;
} MPISearchJob;

bool initMpi(CubeState cube, int length);


#endif