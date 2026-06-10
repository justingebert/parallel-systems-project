#include "cube.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <omp.h>

static size_t numExpandedStates(CubeExpansion expansion) {
    return sizeof(expansion.states) / sizeof(expansion.states[0]);
}

/* Recursive DFS that can be cancelled mid-search. When `cancel` is non-NULL and
 * becomes true (because another thread already found a solution), the recursion
 * unwinds early instead of grinding through the rest of its subtree. This is
 * what lets the parallel searches keep the serial version's early-exit pruning.
 * A relaxed load is sufficient: we only need to stop eventually, not exactly. */
static bool depthFirstSearchCancellable(CubeState cube, int length, atomic_bool *cancel) {
    if (cancel && atomic_load_explicit(cancel, memory_order_relaxed)) {
        return false;
    }

    if (isSolved(cube)) {
        return true;
    }

    if (length == 0) {
        return false;
    }

    CubeExpansion expansion = expand(cube);
    const size_t n = numExpandedStates(expansion);

    for (size_t i = 0; i < n; i++) {
        if (depthFirstSearchCancellable(expansion.states[i], length - 1, cancel)) {
            return true;
        }
    }

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
    const size_t n = numExpandedStates(expansion);

    atomic_bool found = false;

    #pragma omp parallel for schedule(dynamic) shared(found, expansion)
    for (size_t i = 0; i < n; i++) {
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
    const size_t n = numExpandedStates(expansion);

    atomic_bool found = false;

    #pragma omp parallel shared(found, expansion)
    {
        #pragma omp single
        {
            #pragma omp taskloop shared(found, expansion)
            for (size_t i = 0; i < n; i++) {
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
    const size_t n = numExpandedStates(expansion);

    atomic_bool found = false;

    #pragma omp parallel shared(found, expansion)
    {
        #pragma omp single
        {
            for (size_t i = 0; i < n; i++) {
                #pragma omp task firstprivate(i) shared(found, expansion)
                {
                    if (!atomic_load(&found)) {
                        if (depthFirstSearchCancellable(expansion.states[i], length - 1, &found)) {
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
    printf("Num of Cores: %d\n", omp_get_num_procs());
    printf("Num max Threads: %d\n", omp_get_max_threads());
    printf("\n");

    runTest("Serial DFS", depthFirstSearch, cube, SCRAMBLE_LEN);
    runTest("OpenMP parallel for", initParallelDfs, cube, SCRAMBLE_LEN);
    runTest("OpenMP taskloop", initParallelDfsWithTaskloop, cube, SCRAMBLE_LEN);
    runTest("OpenMP manual tasks", initParallelDfsWithManualTasks, cube, SCRAMBLE_LEN);

    return 0;
}
