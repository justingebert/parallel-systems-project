#include "cube.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <omp.h>


static bool depthFirstSearchCancellable(CubeState cube, int length, atomic_bool *cancel) {
    if (cancel && atomic_load(cancel)) {
        return false;
    }

    if (isSolved(cube)) {
        return true;
    }

    if (length == 0) {
        return false;
    }

    CubeExpansion expansion = expand(cube);

    for (size_t i = 0; i < CUBE_MOVE_COUNT; i++) {
        if (depthFirstSearchCancellable(expansion.states[i], length - 1, cancel)) {
            return true;
        }
    }

    return false;
}


static bool depthFirstSearchCancellableOpenMP(CubeState cube, int length, atomic_bool *cancel, int *initialLength) {
    if (cancel && atomic_load(cancel)) {
        return false;
    }

    if (isSolved(cube)) {
        return true;
    }

    if (length == 0) {
        return false;
    }    

    CubeExpansion expansion = expand(cube);
    int cutoff = *initialLength - 2;

    for (size_t i = 0; i < CUBE_MOVE_COUNT; i++) {
            #pragma omp task if(length > cutoff) firstprivate(i) shared(cancel, expansion)
            {
                if (!atomic_load(cancel)) {
                    if (depthFirstSearchCancellableOpenMP(expansion.states[i], length - 1, cancel, initialLength)) {
                        atomic_store(cancel, true);
                    }
                }
            }
        }

    /*
    if (length >= *initialLength - 2)
    {
        for (size_t i = 0; i < CUBE_MOVE_COUNT; i++) {
            #pragma omp task firstprivate(i) shared(cancel, expansion)
            {
                if (!atomic_load(cancel)) {
                    if (depthFirstSearchCancellableOpenMP(expansion.states[i], length - 1, cancel, initialLength)) {
                        atomic_store(cancel, true);
                    }
                }
            }
        }
        #pragma omp taskwait
    } else {
        for (size_t i = 0; i < CUBE_MOVE_COUNT; i++) {
            if (depthFirstSearchCancellableOpenMP(expansion.states[i], length - 1, cancel, initialLength)) {
                return true;
            }
        }
    }
    */

    return false;
}

bool depthFirstSearch(CubeState cube, int length) {
    return depthFirstSearchCancellable(cube, length, NULL);
}

bool initParallelDfs(CubeState cube, int length) {
    if (isSolved(cube)) {
        return true;
    }

    if (length == 0) {
        return false;
    }

    CubeExpansion expansion = expand(cube);

    atomic_bool found = false;

    #pragma omp parallel for schedule(static) default(none) shared(expansion, length, found)
    for (size_t i = 0; i < CUBE_MOVE_COUNT; i++) {
        if (atomic_load(&found)) {
            continue;
        }

        if (depthFirstSearchCancellable(expansion.states[i], length - 1, &found)) {
            atomic_store(&found, true);
        }
    }

    return atomic_load(&found);
}

bool initParallelDfsWithTaskloop(CubeState cube, int length) {
    if (isSolved(cube)) {
        return true;
    }

    if (length == 0) {
        return false;
    }

    CubeExpansion expansion = expand(cube);

    atomic_bool found = false;

    #pragma omp parallel shared(found, expansion)
    {
        #pragma omp single
        {
            #pragma omp taskloop shared(found, expansion)
            for (size_t i = 0; i < CUBE_MOVE_COUNT; i++) {
                if (atomic_load(&found)) {
                    continue;
                }

                if (depthFirstSearchCancellable(expansion.states[i], length - 1, &found)) {
                    atomic_store(&found, true);
                }
            }
        }
    }

    return atomic_load(&found);
}

bool initParallelDfsWithManualTasks(CubeState cube, int length) {
    if (isSolved(cube)) {
        return true;
    }

    if (length == 0) {
        return false;
    }

    CubeExpansion expansion = expand(cube);

    atomic_bool found = false;

    #pragma omp parallel shared(found, expansion)
    {
        #pragma omp single
        {
            for (size_t i = 0; i < CUBE_MOVE_COUNT; i++) {
                #pragma omp task firstprivate(i) shared(found, expansion)
                {
                    if (!atomic_load(&found)) {
                        if (depthFirstSearchCancellableOpenMP(expansion.states[i], length - 1, &found, &length)) {
                            atomic_store(&found, true);
                        }
                    }
                }
            }

            #pragma omp taskwait
        }
    }

    return atomic_load(&found);
}

static void runTest(
    const char *name,
    bool (*searchFunction)(CubeState, int),
    CubeState cube,
    int length
) {
    double start = omp_get_wtime();

    bool result = searchFunction(cube, length);

    double end = omp_get_wtime();

    printf("%-30s result: %-6s time: %.6f seconds\n",
           name,
           result ? "solved" : "failed",
           end - start);
}

int main(void) {
    enum { SCRAMBLE_LEN = 8 };
    const uint32_t seed = 20260602u;

    srand(seed);

    CubeState cube = scramble(SOLVED, SCRAMBLE_LEN);

    printf("Used Seed: %u\n", seed);
    printf("Num of Scrambles: %d\n", SCRAMBLE_LEN);
    printf("Number of Nodes in Search Tree: %zu\n", (size_t)pow(CUBE_MOVE_COUNT, SCRAMBLE_LEN));
    printf("Num of Cores: %d\n", omp_get_num_procs());
    printf("Num max Threads: %d\n", omp_get_max_threads());
    printf("\n");

    // runTest("Serial DFS", depthFirstSearch, cube, SCRAMBLE_LEN);
    // runTest("OpenMP parallel for", initParallelDfs, cube, SCRAMBLE_LEN);
    runTest("OpenMP taskloop", initParallelDfsWithTaskloop, cube, SCRAMBLE_LEN);
    runTest("OpenMP manual tasks", initParallelDfsWithManualTasks, cube, SCRAMBLE_LEN);

    return 0;
}
