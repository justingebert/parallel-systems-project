#include "cube.h"
#include "solve.h"
#include "benchmark.h"
#include "cJSON.h"

#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>

BenchmarkResult benchmarkAlgorithm(SolveFn algorithm, const char *technology, const char *algorithm_name, BenchmarkConfig config) {
    omp_set_num_threads(config.numCores);

    int repeats = config.repeats;
    if (repeats > BENCHMARK_MAX_REPEATS) {
        repeats = BENCHMARK_MAX_REPEATS;
    }

    double total = 0.0;
    double min = 0.0;
    double max = 0.0;
    int solvedCount = 0;

    BenchmarkResult result = {
        .technology = technology,
        .algorithm = algorithm_name,
        .runCount = repeats
    };

    for (int i = 0; i < repeats; ++i) {
        srand(config.seed); // needed so that every run has the same initial cube

        CubeState cube = scramble(SOLVED, config.scrambleLen);

        double start = omp_get_wtime();
        bool solved = algorithm(cube, config.scrambleLen);
        double elapsed = omp_get_wtime() - start;

        result.runs[i].seconds = elapsed;
        result.runs[i].solved = solved;

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

    result.avgSeconds = total / repeats;
    result.minSeconds = min;
    result.maxSeconds = max;
    result.solvedCount = solvedCount;

    return result;
}

void writeBenchmarkReport(BenchmarkConfig config, const BenchmarkResult *results, int count) {
    char timestamp[32];
    time_t now = time(NULL);
    strftime(timestamp, sizeof timestamp, "%y-%m-%d-%H-%M", localtime(&now));

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "timestamp", timestamp);
    cJSON_AddNumberToObject(root, "seed", config.seed);
    cJSON_AddNumberToObject(root, "scrambleMoves", config.scrambleLen);
    cJSON_AddNumberToObject(root, "cores", config.numCores);
    cJSON_AddNumberToObject(root, "repeats", config.repeats);

    cJSON *resultsArray = cJSON_AddArrayToObject(root, "results");
    for (int i = 0; i < count; ++i) {
        const BenchmarkResult *r = &results[i];

        cJSON *entry = cJSON_CreateObject();
        cJSON_AddStringToObject(entry, "technology", r->technology);
        cJSON_AddStringToObject(entry, "algorithm", r->algorithm);
        cJSON_AddNumberToObject(entry, "avg", r->avgSeconds);
        cJSON_AddNumberToObject(entry, "min", r->minSeconds);
        cJSON_AddNumberToObject(entry, "max", r->maxSeconds);
        cJSON_AddNumberToObject(entry, "solved", r->solvedCount);

        cJSON *runs = cJSON_AddArrayToObject(entry, "runs");
        for (int j = 0; j < r->runCount; ++j) {
            cJSON *run = cJSON_CreateObject();
            cJSON_AddNumberToObject(run, "time", r->runs[j].seconds);
            cJSON_AddBoolToObject(run, "solved", r->runs[j].solved);
            cJSON_AddItemToArray(runs, run);
        }

        cJSON_AddItemToArray(resultsArray, entry);
    }

    if (mkdir("data", 0755) != 0 && errno != EEXIST) {
        perror("mkdir data");
    }

    char path[64];
    snprintf(path, sizeof path, "data/%s.json", timestamp);

    char *json = cJSON_Print(root);
    FILE *file = fopen(path, "w");
    if (file) {
        fputs(json, file);
        fclose(file);
        printf("Wrote benchmark report to %s\n", path);
    } else {
        perror("fopen report");
    }

    cJSON_free(json);
    cJSON_Delete(root);
}