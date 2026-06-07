#include "cube.h"

#include <stdio.h>
#include <stdlib.h>

bool depthFirstSearch(CubeState cube, int length) {
    if(isSolved(cube)){
        return true;
    };

    if (length == 0) {
        return false;
    }
    
    CubeExpansion expansion = expand(cube);
    for (long unsigned int i = 0; i < (sizeof(expansion.states) / sizeof(expansion.states[0])); i++) {
        if (depthFirstSearch(expansion.states[i], length - 1)) {
            return true;
        }
    }

    return false;
}

int main(void)
{   

    enum { SCRAMBLE_LEN = 3 };
    const uint32_t seed = 20260602u;
    srand(seed);

    CubeState cube = scramble(SOLVED, SCRAMBLE_LEN);

    printf("Check cube state: %s\n",  depthFirstSearch(cube, SCRAMBLE_LEN) ? "solved" : "failed");
    return isSolved(cube) ? 0 : 1;
}
