#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "cube.h"
#include "solve.h"

#define BENCHMARK_MAX_REPEATS 100

typedef struct {
    int scrambleLen;
    uint32_t seed;
    int numCores;
    int repeats;
} BenchmarkConfig;

typedef struct {
    double seconds;
    bool solved;
} BenchmarkRun;

typedef struct {
    const char *technology;
    const char *algorithm;
    double avgSeconds;
    double minSeconds;
    double maxSeconds;
    int solvedCount;
    int runCount;
    BenchmarkRun runs[BENCHMARK_MAX_REPEATS];
} BenchmarkResult;

BenchmarkResult benchmarkAlgorithm(SolveFn algorithm, const char *technology, const char *algorithm_name, BenchmarkConfig config);

void writeBenchmarkReport(BenchmarkConfig config, const BenchmarkResult *results, int count);

#endif