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
        .scrambleLen = 6,
        .seeds = seeds,
        .seedCount = (int)(sizeof(seeds) / sizeof(seeds[0])),
        .coreCounts = coreCounts,
        .coreCountCount = (int)(sizeof(coreCounts) / sizeof(coreCounts[0])),
        .repeats = 10
    };

    struct {
        const char *technology;
        const char *algorithm;
        SolveFn fn;
        bool parallel;
    } algos[] = {
        {"serial", "baseline",     depthFirstSearch,             false},
        {"OpenMP", "for_static",   initParallelDfsStatic,        true},
        {"OpenMP", "for_dynamic",  initParallelDfsDynamic,       true},
        {"OpenMP", "for_guided",   initParallelDfsGuided,        true},
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

            printf("Result: avg=%.6fs cores=%d technology=%s algorithm=%s\n",
               results[n - 1].avgSeconds, results[n - 1].cores,
               results[n - 1].technology, results[n - 1].algorithm);
        }
    }
    writeBenchmarkReport(config, results, n, NULL);

    /* Vary the taskgroup spawn depth */
    int maxCores = config.coreCounts[config.coreCountCount - 1];
    BenchmarkResult granResults[config.scrambleLen];
    int gn = 0;
    for (int depth = 1; depth <= config.scrambleLen; ++depth) {
        setTaskSpawnDepth(depth);
        BenchmarkResult r = benchmarkAlgorithm(initParallelDfsWithTaskgroup, "OpenMP",
                                               "taskgroup", maxCores, config);
        r.spawnDepth = depth;
        granResults[gn++] = r;

        printf("Granularity: avg=%.6fs depth=%d cores=%d\n", r.avgSeconds, depth, maxCores);
    }
    writeBenchmarkReport(config, granResults, gn, "granularity");

    return 0;
}
