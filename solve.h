#ifndef SOLVE_H
#define SOLVE_H

#include "cube.h"

typedef bool (*SolveFn)(CubeState cube, int length);

bool depthFirstSearch(CubeState cube, int length);
bool initParallelDfs(CubeState cube, int length);
bool initParallelDfsWithTaskloop(CubeState cube, int length);
bool initParallelDfsWithManualTasks(CubeState cube, int length);

#endif