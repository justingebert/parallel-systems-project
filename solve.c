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

    #pragma omp parallel for schedule(runtime) default(none) shared(expansion, length, found)
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


bool initParallelDfsStatic(CubeState cube, int length) {
    omp_set_schedule(omp_sched_static, 0);   /* 0 = even contiguous blocks, 2 = round robin */
    return initParallelDfs(cube, length);
}


bool initParallelDfsDynamic(CubeState cube, int length) {
    omp_set_schedule(omp_sched_dynamic, 1);  /* 1 = grab one subtree at a time */
    return initParallelDfs(cube, length);
}


bool initParallelDfsGuided(CubeState cube, int length) {
    omp_set_schedule(omp_sched_guided, 0);   /* large chunks that shrink -> middle ground */
    return initParallelDfs(cube, length);
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


static void searchTaskgroup(CubeState cube, int length, int cutoff, atomic_bool *found) {
    if (atomic_load(found)) {
        return;
    }

    if (isSolved(cube)) {
        atomic_store(found, true);
        return;
    }

    if (length == 0) {
        return;
    }

    CubeExpansion expansion = expand(cube);

    if (length > cutoff) {
        for (size_t i = 0; i < CUBE_MOVE_COUNT; i++) {
            CubeState child = expansion.states[i];
            #pragma omp task default(none) firstprivate(child, length, cutoff) shared(found)
            searchTaskgroup(child, length - 1, cutoff, found);
        }
        /* no taskwait: the taskgroup in initParallelDfsWithTaskgroup waits for every descendant at once. */
    } else {
        for (size_t i = 0; i < CUBE_MOVE_COUNT; i++) {
            if (atomic_load(found)) {
                return;
            }
            searchTaskgroup(expansion.states[i], length - 1, cutoff, found);
        }
    }
}


static void searchTaskwait(CubeState cube, int length, int cutoff, atomic_bool *found) {
    if (atomic_load(found)) {
        return;
    }

    if (isSolved(cube)) {
        atomic_store(found, true);
        return;
    }

    if (length == 0) {
        return;
    }

    CubeExpansion expansion = expand(cube);

    if (length > cutoff) {
        for (size_t i = 0; i < CUBE_MOVE_COUNT; i++) {
            CubeState child = expansion.states[i];
            #pragma omp task default(none) firstprivate(child, length, cutoff) shared(found)
            searchTaskwait(child, length - 1, cutoff, found);
        }
        #pragma omp taskwait /* fork-join: block until this level's children finish */
    } else {
        for (size_t i = 0; i < CUBE_MOVE_COUNT; i++) {
            if (atomic_load(found)) {
                return;
            }
            searchTaskwait(expansion.states[i], length - 1, cutoff, found);
        }
    }
}

static int g_spawnDepth = 2;

void setTaskSpawnDepth(int n) { g_spawnDepth = n; }


bool initParallelDfsWithTaskgroup(CubeState cube, int length) {
    atomic_bool found = false;

    /* Lower cutoff -> more, smaller tasks. */
    const int cutoff = length - g_spawnDepth;

    #pragma omp parallel shared(found)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                searchTaskgroup(cube, length, cutoff, &found);
            }
        }
    }

    return atomic_load(&found);
}


bool initParallelDfsWithTaskwait(CubeState cube, int length) {
    atomic_bool found = false;

    const int cutoff = length - 2;

    #pragma omp parallel shared(found)
    {
        #pragma omp single
        {
            searchTaskwait(cube, length, cutoff, &found);
        }
    }

    return atomic_load(&found);
}
