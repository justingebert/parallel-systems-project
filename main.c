#include "cube.h"
#include "solve.h"
#include "benchmark.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <omp.h>


/*
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
*/


int main(void) {
    /*
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
    // runTest("OpenMP taskloop", initParallelDfsWithTaskloop, cube, SCRAMBLE_LEN);
    runTest("OpenMP manual tasks", initParallelDfsWithManualTasks, cube, SCRAMBLE_LEN);
    */
    
    BenchmarkConfig config = {
        .scrambleLen = 8,
        .seed = 20260602u,
        .numCores = 16,
        .repeats = 5,
        .technology = "OpenMP",
        .algorithm = "taskloop"
    };
    BenchmarkResult result = benchmarkAlgorithm(initParallelDfsWithManualTasks, config);
    printf("Result: avg=%.6fs min=%.6fs max=%.6fs solved=%d/%d technology=%s algorithm=%s\n", result.avgSeconds, result.minSeconds, result.maxSeconds, result.solvedCount, config.repeats, result.technology, result.algorithm);
    
    return 0;
}
