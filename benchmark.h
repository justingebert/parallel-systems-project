#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "cube.h"
#include "solve.h"

#define BENCHMARK_MAX_REPEATS 100

typedef struct {
    int scrambleLen;
    const uint32_t *seeds;  /* each seed -> one scramble, run `repeats` times */
    int seedCount;
    int numCores;
    int repeats;            /* repeats per seed */
} BenchmarkConfig;

typedef struct {
    double seconds;
    bool solved;
    uint32_t seed;
} BenchmarkRun;

typedef struct {
    const char *technology;
    const char *algorithm;
    int runCount;
    double avgSeconds;
    BenchmarkRun runs[BENCHMARK_MAX_REPEATS];
} BenchmarkResult;

BenchmarkResult benchmarkAlgorithm(SolveFn algorithm, const char *technology, const char *algorithm_name, BenchmarkConfig config);

void writeBenchmarkReport(BenchmarkConfig config, const BenchmarkResult *results, int count);

#endif