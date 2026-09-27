#ifndef COMMON_H
#define COMMON_H

#include "include_asm.h"
#include "types.h"

/* The byte offset of `member` in `type`: C89's <stddef.h> offsetof, which
 * Sony's stddef.h does not provide. A constant expression to cc1. */
#ifndef offsetof
#define offsetof(type, member) ((unsigned int)&((type *)0)->member)
#endif

/* The number of elements in the array `arr` (an array, never a pointer).
 * Signed, so `i < ARRAY_COUNT(a)` on an s32 compares signed, as a literal
 * would. */
#define ARRAY_COUNT(arr) ((s32)(sizeof(arr) / sizeof((arr)[0])))

/* `deg` degrees as a PlayStation angle, 4096 (libgte's ONE) to the turn.
 * Exact for multiples of 45 degrees; a constant expression to cc1. */
#define ANGLE_DEG(deg) ((deg) * 4096 / 360)

/* 20.12 fixed point: libgte's ONE is 1 << FIX12_SHIFT, so `n << FIX12_SHIFT`
 * is the integer n as a fixed-point value. */
#define FIX12_SHIFT 12

typedef union MoodGraphPoint {
    s16 value;

    struct axis {
        s8 dynamic;
        s8 upper;
    } axis;
} MoodGraphPoint;

#endif