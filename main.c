#include "cube.h"
#include "solve.h"
#include "benchmark.h"
#include "ida_star.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <omp.h>
#include <mpi.h>

#include "solve_mpi.h"

int main(void) {
    BenchmarkConfig config = {
        .scrambleLen = 8,
        .seed = 20260602u,
        .numCores = 16,
        .repeats = 5
    };

    struct {
        const char *technology;
        const char *algorithm;
        SolveFn fn;
    } algos[] = {
        {"serial", "baseline",     depthFirstSearch},
        {"serial", "ida_star",     initIdaStar},
        {"OpenMP", "parallel_for", initParallelDfs},
        {"OpenMP", "taskloop",     initParallelDfsWithTaskloop},
        {"OpenMP", "taskgroup",    initParallelDfsWithTaskgroup},
        {"OpenMP", "taskwait",     initParallelDfsWithTaskwait},
    };
    const int count = sizeof(algos) / sizeof(algos[0]);

    BenchmarkResult results[count];
    for (int i = 0; i < count; ++i) {
        results[i] = benchmarkAlgorithm(algos[i].fn, algos[i].technology, algos[i].algorithm, config);
        
        printf("Result: avg=%.6fs min=%.6fs max=%.6fs solved=%d/%d technology=%s algorithm=%s\n",
               results[i].avgSeconds, results[i].minSeconds, results[i].maxSeconds,
               results[i].solvedCount, config.repeats, results[i].technology, results[i].algorithm);
    }

    writeBenchmarkReport(config, results, count);

    return 0;
}
