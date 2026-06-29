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
    static const int coreCounts[] = {1, 2, 4, 8, 11};

    BenchmarkConfig config = {
        .scrambleLen = 7,
        .seeds = seeds,
        .seedCount = (int)(sizeof(seeds) / sizeof(seeds[0])),
        .coreCounts = coreCounts,
        .coreCountCount = (int)(sizeof(coreCounts) / sizeof(coreCounts[0])),
        .repeats = 2
    };

    struct {
        const char *technology;
        const char *algorithm;
        SolveFn fn;
        bool parallel;
    } algos[] = {
        {"serial", "baseline",     depthFirstSearch,             false},
        {"OpenMP", "parallel_for", initParallelDfs,              true},
        {"OpenMP", "taskloop",     initParallelDfsWithTaskloop,  true},
        {"OpenMP", "taskgroup",    initParallelDfsWithTaskgroup, true},
        {"OpenMP", "taskwait",     initParallelDfsWithTaskwait,  true},
    };
    const int count = (int)(sizeof(algos) / sizeof(algos[0]));

    BenchmarkResult results[count * config.coreCountCount];
    int n = 0;
    for (int a = 0; a < count; ++a) {
        
        int steps = algos[a].parallel ? config.coreCountCount : 1;

        for (int t = 0; t < steps; ++t) {
            int cores = algos[a].parallel ? config.coreCounts[t] : 1;
            results[n++] = benchmarkAlgorithm(algos[a].fn, algos[a].technology,
                                              algos[a].algorithm, cores, config);
        }
    }

    for (int i = 0; i < n; ++i) {
        printf("Result: avg=%.6fs cores=%d technology=%s algorithm=%s\n",
               results[i].avgSeconds, results[i].cores,
               results[i].technology, results[i].algorithm);
    }

    writeBenchmarkReport(config, results, n);

    return 0;
}
