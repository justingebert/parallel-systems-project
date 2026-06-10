#include "cube.h"

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

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

bool initDfs(CubeState cube, int length) {
    CubeExpansion expansion = expand(cube);

    bool found = false;
    
    #pragma omp parallel for schedule(dynamic) shared(found)
    for (long unsigned int i = 0; i < (sizeof(expansion.states) / sizeof(expansion.states[0])); i++) {
        if (found) continue;
        
        printf("Hello from thread %d of %d\n", omp_get_thread_num(), omp_get_num_threads());

        if (depthFirstSearch(expansion.states[i], length - 1)) {
            found = true;
        }
    }

    return found;
}

int main(void)
{   

    enum { SCRAMBLE_LEN = 8 };
    const uint32_t seed = 20260602u;
    srand(seed);

    CubeState cube = scramble(SOLVED, SCRAMBLE_LEN);
    
    omp_set_num_threads(18);

    printf("Num of Cores: %d\n", omp_get_num_procs());
    printf("Num max Threads: %d\n", omp_get_max_threads());

    bool result = initDfs(cube, SCRAMBLE_LEN);
    //bool result = depthFirstSearch(cube, SCRAMBLE_LEN);

    printf("Check cube state: %s\n", result ? "solved" : "failed");
    return result ? 0 : 1;
}
