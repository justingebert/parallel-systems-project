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

BenchmarkResult benchmarkAlgorithm(SolveFn algorithm, const char *technology,
                                   const char *algorithm_name, int cores, BenchmarkConfig config) {
    omp_set_num_threads(cores);

    BenchmarkResult result = {
        .technology = technology,
        .algorithm = algorithm_name,
        .cores = cores,
        .runCount = 0
    };

    for (int s = 0; s < config.seedCount && result.runCount < BENCHMARK_MAX_REPEATS; ++s) {
        for (int r = 0; r < config.repeats && result.runCount < BENCHMARK_MAX_REPEATS; ++r) {
            srand(config.seeds[s]); // same seed -> same initial cube

            CubeState cube = scramble(SOLVED, config.scrambleLen);

            double start = omp_get_wtime();
            bool solved = algorithm(cube, config.scrambleLen);
            double elapsed = omp_get_wtime() - start;

            int i = result.runCount++;
            result.runs[i].seconds = elapsed;
            result.runs[i].solved = solved;
            result.runs[i].seed = config.seeds[s];

            result.avgSeconds += elapsed;
        }
    }

    if (result.runCount > 0) {
        result.avgSeconds /= result.runCount;
    }

    return result;
}

void writeBenchmarkReport(BenchmarkConfig config, const BenchmarkResult *results, int count, const char *label) {
    char timestamp[32];
    time_t now = time(NULL);
    strftime(timestamp, sizeof timestamp, "%y-%m-%d-%H-%M", localtime(&now));

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "timestamp", timestamp);
    cJSON_AddNumberToObject(root, "scrambleMoves", config.scrambleLen);
    cJSON_AddNumberToObject(root, "repeats", config.repeats);

    cJSON *seeds = cJSON_AddArrayToObject(root, "seeds");
    for (int i = 0; i < config.seedCount; ++i) {
        cJSON_AddItemToArray(seeds, cJSON_CreateNumber(config.seeds[i]));
    }

    cJSON *resultsArray = cJSON_AddArrayToObject(root, "results");
    for (int i = 0; i < count; ++i) {
        const BenchmarkResult *r = &results[i];

        cJSON *entry = cJSON_CreateObject();
        cJSON_AddStringToObject(entry, "technology", r->technology);
        cJSON_AddStringToObject(entry, "algorithm", r->algorithm);
        cJSON_AddNumberToObject(entry, "cores", r->cores);
        cJSON_AddNumberToObject(entry, "spawnDepth", r->spawnDepth);

        cJSON *runs = cJSON_AddArrayToObject(entry, "runs");
        for (int j = 0; j < r->runCount; ++j) {
            cJSON *run = cJSON_CreateObject();
            cJSON_AddNumberToObject(run, "time", r->runs[j].seconds);
            cJSON_AddBoolToObject(run, "solved", r->runs[j].solved);
            cJSON_AddNumberToObject(run, "seed", r->runs[j].seed);
            cJSON_AddItemToArray(runs, run);
        }

        cJSON_AddItemToArray(resultsArray, entry);
    }

    if (mkdir("data", 0755) != 0 && errno != EEXIST) {
        perror("mkdir data");
    }

    char path[64];
    if (label && *label) {
        snprintf(path, sizeof path, "data/%s-%s.json", timestamp, label);
    } else {
        snprintf(path, sizeof path, "data/%s.json", timestamp);
    }

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