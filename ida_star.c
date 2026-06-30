#include "ida_star.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>


// estimates the number of moves needed to solve the cube state by counting the number of corners and edges that are not solved
static int heuristic(CubeState cube) {
    // Corner sticker triplets
    static const int corners[8][3] = {
        {  8,  9, 20 }, {  6, 18, 38 },
        {  0, 36, 47 }, {  2, 45, 11 },
        { 29, 26, 15 }, { 27, 44, 24 },
        { 33, 53, 42 }, { 35, 17, 51 },
    };

    // Edge sticker pairs
    static const int edges[12][2] = {
        {  1, 46 }, {  5, 10 }, {  7, 19 }, {  3, 37 },
        { 39, 50 }, { 41, 21 }, { 23, 12 }, { 14, 48 },
        { 16, 32 }, { 28, 25 }, { 30, 43 }, { 34, 52 },
    };

    // go through all corners and check which ones are in the wrong position
    int wrong_corners = 0;
    
    for (int i = 0; i < 8; i++) {
        for (int k = 0; k < 3; k++) {
            int idx = corners[i][k];
            if (cube.stickers[idx] != SOLVED.stickers[idx]) {
                wrong_corners++;
                break; // count whole corner as 1, not 3 for each sticker
            }
        }
    }

    // go through all edges and check which ones are in the wrong position
    int wrong_edges = 0;

    for (int i = 0; i < 12; i++) {
        for (int k = 0; k < 2; k++) {
            int idx = edges[i][k];
            if (cube.stickers[idx] != SOLVED.stickers[idx]) {
                wrong_edges++;
                break; // count whole edge as 1, not 2 for each sticker
            }
        }
    }

    // one move can fix at most four corners/edges in the best case scenario
    //   -> underestimate the number of moves needed to not miss a solution
    if(wrong_corners > wrong_edges) {
        return wrong_corners / 4; 
    } else {
        return wrong_edges / 4;
    }
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