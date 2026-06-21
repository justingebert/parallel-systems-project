#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "cube.h"
#include "solve.h"

typedef struct {
    int scrambleLen;
    uint32_t seed;
    int numCores;
    int repeats;
    const char *technology;
    const char *algorithm;
} BenchmarkConfig;

typedef struct {
    double avgSeconds;
    double minSeconds;
    double maxSeconds;
    int solvedCount;
    const char *technology;
    const char *algorithm;
} BenchmarkResult;

BenchmarkResult benchmarkAlgorithm(SolveFn algorithm, BenchmarkConfig config);

#endif