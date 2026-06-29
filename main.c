#include "cube.h"
#include "solve.h"
#include "benchmark.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <omp.h>

int main(void) {
    static const uint32_t seeds[] = {20260602u, 123u, 777u, 2024u, 42u};

    BenchmarkConfig config = {
        .scrambleLen = 8,
        .seeds = seeds,
        .seedCount = (int)(sizeof(seeds) / sizeof(seeds[0])),
        .numCores = 11,
        .repeats = 2
    };

    struct {
        const char *technology;
        const char *algorithm;
        SolveFn fn;
    } algos[] = {
        {"serial", "baseline",     depthFirstSearch},
        {"OpenMP", "parallel_for", initParallelDfs},
        {"OpenMP", "taskloop",     initParallelDfsWithTaskloop},
        {"OpenMP", "taskgroup",    initParallelDfsWithTaskgroup},
        {"OpenMP", "taskwait",     initParallelDfsWithTaskwait},
    };
    const int count = sizeof(algos) / sizeof(algos[0]);

    BenchmarkResult results[count];
    for (int i = 0; i < count; ++i) {
        results[i] = benchmarkAlgorithm(algos[i].fn, algos[i].technology, algos[i].algorithm, config);

        printf("Result: avg=%.6fs technology=%s algorithm=%s\n",
               results[i].avgSeconds, results[i].technology, results[i].algorithm);
    }

    writeBenchmarkReport(config, results, count);

    return 0;
}
