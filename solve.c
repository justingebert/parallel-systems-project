#include "solve.h"
 
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


static bool depthFirstSearchCancellableManualTasks(CubeState cube, int length, atomic_bool *cancel, int *initialLength) {
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

    // discard the task if approach: it still spawns tasks until the deepest level, which introduces a huge overhead
    /*
    for (size_t i = 0; i < CUBE_MOVE_COUNT; i++) 
    {
        #pragma omp task if(length > cutoff) firstprivate(i, expansion) shared(cancel, initialLength)
        {
            if (!atomic_load(cancel)) {
                if (depthFirstSearchCancellableManualTasks(expansion.states[i], length - 1, cancel, initialLength)) {
                    atomic_store(cancel, true);
                }
            }
        }
    }
    #pragma omp taskwait
    */
    
    if (length > cutoff)
    {
        for (size_t i = 0; i < CUBE_MOVE_COUNT; i++) {
            #pragma omp task firstprivate(i, expansion) shared(cancel, initialLength) // need expansion in firstprivate, sharing the object (apparently) introduces a race condition
            {
                if (!atomic_load(cancel)) {
                    if (depthFirstSearchCancellableManualTasks(expansion.states[i], length - 1, cancel, initialLength)) {
                        atomic_store(cancel, true);
                    }
                }
            }
        }
        #pragma omp taskwait // important since the function discards child results otherwise - thats why we got solved=false sometimes
    } else {
        for (size_t i = 0; i < CUBE_MOVE_COUNT; i++) {
            if (atomic_load(cancel)) {
                return false;
            }
            if (depthFirstSearchCancellableManualTasks(expansion.states[i], length - 1, cancel, initialLength)) {
                return true;
            }
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
                        if (depthFirstSearchCancellableManualTasks(expansion.states[i], length - 1, &found, &length)) {
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