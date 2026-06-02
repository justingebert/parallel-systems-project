#ifndef CUBE_H
#define CUBE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CUBE_STICKERS 54
#define CUBE_MOVE_COUNT 18

typedef struct {
    uint8_t stickers[CUBE_STICKERS];
} CubeState;

_Static_assert(sizeof(CubeState) == CUBE_STICKERS,
               "CubeState must stay a flat 54-byte value");

typedef enum {
    MOVE_U,
    MOVE_U2,
    MOVE_U_PRIME,
    MOVE_R,
    MOVE_R2,
    MOVE_R_PRIME,
    MOVE_F,
    MOVE_F2,
    MOVE_F_PRIME,
    MOVE_D,
    MOVE_D2,
    MOVE_D_PRIME,
    MOVE_L,
    MOVE_L2,
    MOVE_L_PRIME,
    MOVE_B,
    MOVE_B2,
    MOVE_B_PRIME
} CubeMove;

typedef struct {
    CubeState states[CUBE_MOVE_COUNT];
} CubeExpansion;

extern const CubeState SOLVED;
extern const uint8_t MOVE_PERMS[CUBE_MOVE_COUNT][CUBE_STICKERS];

CubeState applyMove(CubeState state, CubeMove move);
bool isSolved(CubeState state);
CubeState scramble(CubeState state, size_t n, uint32_t seed);
CubeExpansion expand(CubeState state);

CubeMove inverseMove(CubeMove move);
CubeMove scrambleMove(size_t step, uint32_t seed);
const char *moveName(CubeMove move);

#endif
