#include "cube.h"
#include "solve.h"
#include "benchmark.h"

#include <omp.h>
#include <stdlib.h>
#include <stdio.h>

BenchmarkResult benchmarkAlgorithm(SolveFn algorithm, BenchmarkConfig config) {
    omp_set_num_threads(config.numCores);

    double total = 0.0;
    double min = 0.0;
    double max = 0.0;
    int solvedCount = 0;

    for (int i = 0; i < config.repeats; ++i) {
        srand(config.seed); // needed so that every run has the same initial cube
        
        CubeState cube = scramble(SOLVED, config.scrambleLen);

        double start = omp_get_wtime();
        bool solved = algorithm(cube, config.scrambleLen);
        double elapsed = omp_get_wtime() - start;

        if (solved) {
            solvedCount++;
        }

        total += elapsed;
        if (i == 0 || elapsed < min) {
            min = elapsed;
        }
        if (i == 0 || elapsed > max) {
            max = elapsed;   
        }
    }

    return (BenchmarkResult){
        .avgSeconds = total / config.repeats,
        .minSeconds = min,
        .maxSeconds = max,
        .solvedCount = solvedCount,
        .technology = config.technology,
        .algorithm = config.algorithm
    };
}