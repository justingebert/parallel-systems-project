#include "ida_star.h"
#include <limits.h>
#include <stdio.h>


// estimates the number of moves needed to solve the cube state by counting wrong stickers and dividing by 9 (the number of stickers per face)
static int heuristic(CubeState cube) {
    int wrong = 0;

    for (int i = 0; i < CUBE_STICKERS; ++i) {
        if (cube.stickers[i] != SOLVED.stickers[i]) {
            wrong += 1;
        }
    }

    return wrong / 9;
}

/* 
    A* part
    cube: current state of cube
    movesTaken: number of moves taken to reach this state
    bound: cutoff depth for this iteration of the search
*/
static bool search(CubeState cube, int movesTaken, int bound) {
    // if the number of moves taken plus the heuristic estimate for still needed moves exceeds the bound, stop exploring this branch
    if (movesTaken + heuristic(cube) > bound) {
        return false;
    }
    
    if (isSolved(cube)) {
        return true;
    }

    // explore the branches for all possible moves from the current state, incrementing the number of moves taken by 1
    CubeExpansion expansion = expand(cube);
    for (size_t i = 0; i < CUBE_MOVE_COUNT; i++) {
        if (search(expansion.states[i], movesTaken + 1, bound)){
            return true;
        }
    }
    return false;
}

// Iterative Deepening part
bool initIdaStar(CubeState cube, int length) {
    // estimate moves needed to solve initial state, and increase by 1 until a solution is found
    for (int bound = heuristic(cube); bound <= length; bound++) {
        if (search(cube, 0, bound)) {
            return true;
        }
    }
    return false;
}