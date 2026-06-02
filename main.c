#include "cube.h"

#include <stdio.h>

int main(void)
{
    enum { SCRAMBLE_LEN = 20 };
    const uint32_t seed = 20260602u;
    CubeMove moves[SCRAMBLE_LEN];

    for (size_t i = 0; i < SCRAMBLE_LEN; ++i) {
        moves[i] = scrambleMove(i, seed);
    }

    CubeState cube = scramble(SOLVED, SCRAMBLE_LEN, seed);

    printf("Scramble:");
    for (size_t i = 0; i < SCRAMBLE_LEN; ++i) {
        printf(" %s", moveName(moves[i]));
    }
    printf("\n");

    for (size_t i = SCRAMBLE_LEN; i > 0; --i) {
        cube = applyMove(cube, inverseMove(moves[i - 1]));
    }

    printf("Inverse check: %s\n", isSolved(cube) ? "solved" : "failed");

    return isSolved(cube) ? 0 : 1;
}
