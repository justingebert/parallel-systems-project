#include "cube.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

enum {
    COLOR_U,
    COLOR_R,
    COLOR_F,
    COLOR_D,
    COLOR_L,
    COLOR_B
};

const CubeState SOLVED = {
    .stickers = {
        COLOR_U, COLOR_U, COLOR_U, COLOR_U, COLOR_U, COLOR_U, COLOR_U, COLOR_U, COLOR_U,
        COLOR_R, COLOR_R, COLOR_R, COLOR_R, COLOR_R, COLOR_R, COLOR_R, COLOR_R, COLOR_R,
        COLOR_F, COLOR_F, COLOR_F, COLOR_F, COLOR_F, COLOR_F, COLOR_F, COLOR_F, COLOR_F,
        COLOR_D, COLOR_D, COLOR_D, COLOR_D, COLOR_D, COLOR_D, COLOR_D, COLOR_D, COLOR_D,
        COLOR_L, COLOR_L, COLOR_L, COLOR_L, COLOR_L, COLOR_L, COLOR_L, COLOR_L, COLOR_L,
        COLOR_B, COLOR_B, COLOR_B, COLOR_B, COLOR_B, COLOR_B, COLOR_B, COLOR_B, COLOR_B
    }
};

/*
 * Facelet indexing
 * ----------------
 * The 54 stickers are grouped by face in this order:
 *
 *   U:  0.. 8   R:  9..17   F: 18..26
 *   D: 27..35   L: 36..44   B: 45..53
 *
 * Within each face, stickers are row-major as viewed from outside the cube:
 *
 *   0 1 2
 *   3 4 5
 *   6 7 8
 *
 * MOVE_PERMS
 * ----------
 * MOVE_PERMS[move][dst] gives the source sticker copied into destination dst.
 *
 * applyMove() therefore does:
 *
 *   next.stickers[dst] = state.stickers[MOVE_PERMS[move][dst]]
 *
 * Example: for MOVE_U, the first entry is 6, so destination sticker 0 receives
 * old sticker 6. That rotates the U face clockwise: old bottom-left moves to
 * new top-left. The side-face entries in the same row rotate the top edge ring.
 *
 * Each initializer below is formatted as six lines of nine numbers. Those six
 * lines match the face order U, R, F, D, L, B, which makes it easier to see
 * which face positions a move changes.
 *
 * This destination-to-source form is useful because each output sticker is
 * written exactly once, with no temporary swaps and no mutation of the input.
 */
const uint8_t MOVE_PERMS[CUBE_MOVE_COUNT][CUBE_STICKERS] = {
    /* U */
    {
         6,  3,  0,  7,  4,  1,  8,  5,  2,
        45, 46, 47, 12, 13, 14, 15, 16, 17,
         9, 10, 11, 21, 22, 23, 24, 25, 26,
        27, 28, 29, 30, 31, 32, 33, 34, 35,
        18, 19, 20, 39, 40, 41, 42, 43, 44,
        36, 37, 38, 48, 49, 50, 51, 52, 53
    },
    /* U2 */
    {
         8,  7,  6,  5,  4,  3,  2,  1,  0,
        36, 37, 38, 12, 13, 14, 15, 16, 17,
        45, 46, 47, 21, 22, 23, 24, 25, 26,
        27, 28, 29, 30, 31, 32, 33, 34, 35,
         9, 10, 11, 39, 40, 41, 42, 43, 44,
        18, 19, 20, 48, 49, 50, 51, 52, 53
    },
    /* U' */
    {
         2,  5,  8,  1,  4,  7,  0,  3,  6,
        18, 19, 20, 12, 13, 14, 15, 16, 17,
        36, 37, 38, 21, 22, 23, 24, 25, 26,
        27, 28, 29, 30, 31, 32, 33, 34, 35,
        45, 46, 47, 39, 40, 41, 42, 43, 44,
         9, 10, 11, 48, 49, 50, 51, 52, 53
    },
    /* R */
    {
         0,  1, 20,  3,  4, 23,  6,  7, 26,
        15, 12,  9, 16, 13, 10, 17, 14, 11,
        18, 19, 29, 21, 22, 32, 24, 25, 35,
        27, 28, 51, 30, 31, 48, 33, 34, 45,
        36, 37, 38, 39, 40, 41, 42, 43, 44,
         8, 46, 47,  5, 49, 50,  2, 52, 53
    },
    /* R2 */
    {
         0,  1, 29,  3,  4, 32,  6,  7, 35,
        17, 16, 15, 14, 13, 12, 11, 10,  9,
        18, 19, 51, 21, 22, 48, 24, 25, 45,
        27, 28,  2, 30, 31,  5, 33, 34,  8,
        36, 37, 38, 39, 40, 41, 42, 43, 44,
        26, 46, 47, 23, 49, 50, 20, 52, 53
    },
    /* R' */
    {
         0,  1, 51,  3,  4, 48,  6,  7, 45,
        11, 14, 17, 10, 13, 16,  9, 12, 15,
        18, 19,  2, 21, 22,  5, 24, 25,  8,
        27, 28, 20, 30, 31, 23, 33, 34, 26,
        36, 37, 38, 39, 40, 41, 42, 43, 44,
        35, 46, 47, 32, 49, 50, 29, 52, 53
    },
    /* F */
    {
         0,  1,  2,  3,  4,  5, 44, 41, 38,
         6, 10, 11,  7, 13, 14,  8, 16, 17,
        24, 21, 18, 25, 22, 19, 26, 23, 20,
        15, 12,  9, 30, 31, 32, 33, 34, 35,
        36, 37, 27, 39, 40, 28, 42, 43, 29,
        45, 46, 47, 48, 49, 50, 51, 52, 53
    },
    /* F2 */
    {
         0,  1,  2,  3,  4,  5, 29, 28, 27,
        44, 10, 11, 41, 13, 14, 38, 16, 17,
        26, 25, 24, 23, 22, 21, 20, 19, 18,
         8,  7,  6, 30, 31, 32, 33, 34, 35,
        36, 37, 15, 39, 40, 12, 42, 43,  9,
        45, 46, 47, 48, 49, 50, 51, 52, 53
    },
    /* F' */
    {
         0,  1,  2,  3,  4,  5,  9, 12, 15,
        29, 10, 11, 28, 13, 14, 27, 16, 17,
        20, 23, 26, 19, 22, 25, 18, 21, 24,
        38, 41, 44, 30, 31, 32, 33, 34, 35,
        36, 37,  8, 39, 40,  7, 42, 43,  6,
        45, 46, 47, 48, 49, 50, 51, 52, 53
    },
    /* D */
    {
         0,  1,  2,  3,  4,  5,  6,  7,  8,
         9, 10, 11, 12, 13, 14, 24, 25, 26,
        18, 19, 20, 21, 22, 23, 42, 43, 44,
        33, 30, 27, 34, 31, 28, 35, 32, 29,
        36, 37, 38, 39, 40, 41, 51, 52, 53,
        45, 46, 47, 48, 49, 50, 15, 16, 17
    },
    /* D2 */
    {
         0,  1,  2,  3,  4,  5,  6,  7,  8,
         9, 10, 11, 12, 13, 14, 42, 43, 44,
        18, 19, 20, 21, 22, 23, 51, 52, 53,
        35, 34, 33, 32, 31, 30, 29, 28, 27,
        36, 37, 38, 39, 40, 41, 15, 16, 17,
        45, 46, 47, 48, 49, 50, 24, 25, 26
    },
    /* D' */
    {
         0,  1,  2,  3,  4,  5,  6,  7,  8,
         9, 10, 11, 12, 13, 14, 51, 52, 53,
        18, 19, 20, 21, 22, 23, 15, 16, 17,
        29, 32, 35, 28, 31, 34, 27, 30, 33,
        36, 37, 38, 39, 40, 41, 24, 25, 26,
        45, 46, 47, 48, 49, 50, 42, 43, 44
    },
    /* L */
    {
        53,  1,  2, 50,  4,  5, 47,  7,  8,
         9, 10, 11, 12, 13, 14, 15, 16, 17,
         0, 19, 20,  3, 22, 23,  6, 25, 26,
        18, 28, 29, 21, 31, 32, 24, 34, 35,
        42, 39, 36, 43, 40, 37, 44, 41, 38,
        45, 46, 33, 48, 49, 30, 51, 52, 27
    },
    /* L2 */
    {
        27,  1,  2, 30,  4,  5, 33,  7,  8,
         9, 10, 11, 12, 13, 14, 15, 16, 17,
        53, 19, 20, 50, 22, 23, 47, 25, 26,
         0, 28, 29,  3, 31, 32,  6, 34, 35,
        44, 43, 42, 41, 40, 39, 38, 37, 36,
        45, 46, 24, 48, 49, 21, 51, 52, 18
    },
    /* L' */
    {
        18,  1,  2, 21,  4,  5, 24,  7,  8,
         9, 10, 11, 12, 13, 14, 15, 16, 17,
        27, 19, 20, 30, 22, 23, 33, 25, 26,
        53, 28, 29, 50, 31, 32, 47, 34, 35,
        38, 41, 44, 37, 40, 43, 36, 39, 42,
        45, 46,  6, 48, 49,  3, 51, 52,  0
    },
    /* B */
    {
        11, 14, 17,  3,  4,  5,  6,  7,  8,
         9, 10, 35, 12, 13, 34, 15, 16, 33,
        18, 19, 20, 21, 22, 23, 24, 25, 26,
        27, 28, 29, 30, 31, 32, 36, 39, 42,
         2, 37, 38,  1, 40, 41,  0, 43, 44,
        51, 48, 45, 52, 49, 46, 53, 50, 47
    },
    /* B2 */
    {
        35, 34, 33,  3,  4,  5,  6,  7,  8,
         9, 10, 42, 12, 13, 39, 15, 16, 36,
        18, 19, 20, 21, 22, 23, 24, 25, 26,
        27, 28, 29, 30, 31, 32,  2,  1,  0,
        17, 37, 38, 14, 40, 41, 11, 43, 44,
        53, 52, 51, 50, 49, 48, 47, 46, 45
    },
    /* B' */
    {
        42, 39, 36,  3,  4,  5,  6,  7,  8,
         9, 10,  0, 12, 13,  1, 15, 16,  2,
        18, 19, 20, 21, 22, 23, 24, 25, 26,
        27, 28, 29, 30, 31, 32, 17, 14, 11,
        33, 37, 38, 34, 40, 41, 35, 43, 44,
        47, 50, 53, 46, 49, 52, 45, 48, 51
    }
};

CubeState applyMove(CubeState state, CubeMove move)
{
    /* Start from a copy so invalid moves can safely return the input state. */
    CubeState next = state;

    if (move < 0 || move >= CUBE_MOVE_COUNT) {
        return next;
    }

    /* For each destination sticker, copy the sticker from its precomputed
     * source position.
     */
    for (size_t i = 0; i < CUBE_STICKERS; ++i) {
        next.stickers[i] = state.stickers[MOVE_PERMS[move][i]];
    }

    return next;
}

bool isSolved(CubeState state)
{
    return memcmp(state.stickers, SOLVED.stickers, sizeof(state.stickers)) == 0;
}

CubeMove scrambleMove()
{   
    return (CubeMove)(rand() % CUBE_MOVE_COUNT);
}

CubeState scramble(CubeState state, size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        state = applyMove(state, scrambleMove());
    }

    return state;
}

/* retuns all 18 states reachable by one move from the input state. */
CubeExpansion expand(CubeState state)
{
    CubeExpansion expansion;

    for (CubeMove move = MOVE_U; move < CUBE_MOVE_COUNT; ++move) {
        expansion.states[move] = applyMove(state, move);
    }

    return expansion;
}

CubeMove inverseMove(CubeMove move)
{
    static const CubeMove inverses[CUBE_MOVE_COUNT] = {
        MOVE_U_PRIME, MOVE_U2, MOVE_U,
        MOVE_R_PRIME, MOVE_R2, MOVE_R,
        MOVE_F_PRIME, MOVE_F2, MOVE_F,
        MOVE_D_PRIME, MOVE_D2, MOVE_D,
        MOVE_L_PRIME, MOVE_L2, MOVE_L,
        MOVE_B_PRIME, MOVE_B2, MOVE_B
    };

    if (move < 0 || move >= CUBE_MOVE_COUNT) {
        return move;
    }

    return inverses[move];
}

const char *moveName(CubeMove move)
{
    static const char *const names[CUBE_MOVE_COUNT] = {
        "U", "U2", "U'",
        "R", "R2", "R'",
        "F", "F2", "F'",
        "D", "D2", "D'",
        "L", "L2", "L'",
        "B", "B2", "B'"
    };

    if (move < 0 || move >= CUBE_MOVE_COUNT) {
        return "NONO!";
    }

    return names[move];
}
