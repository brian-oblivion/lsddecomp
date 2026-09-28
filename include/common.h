#ifndef COMMON_H
#define COMMON_H

/**
 * @file common.h
 * @brief The header every unit includes first: the fixed-width types
 *        (types.h), the macros that splice a function's disassembly into
 *        a unit, a few
 *        C89 helper macros, and the dream chart's point type.
 */

#include "include_asm.h"
#include "types.h"

/** The byte offset of `member` in `type`: C89's <stddef.h> offsetof, which
 * Sony's stddef.h does not provide. A constant expression. */
#ifndef offsetof
#define offsetof(type, member) ((unsigned int)&((type *)0)->member)
#endif

/** The number of elements in the array `arr` (an array, never a pointer).
 * Signed, so `i < ARRAY_COUNT(a)` on an s32 compares signed, as a literal
 * would. */
#define ARRAY_COUNT(arr) ((s32)(sizeof(arr) / sizeof((arr)[0])))

/** `deg` degrees as a PlayStation angle, 4096 (libgte's ONE) to the turn.
 * Exact for multiples of 45 degrees; a constant expression. */
#define ANGLE_DEG(deg) ((deg) * 4096 / 360)

/** 20.12 fixed point: libgte's ONE is 1 << FIX12_SHIFT, so `n << FIX12_SHIFT`
 * is the integer n as a fixed-point value. */
#define FIX12_SHIFT 12

/**
 * @brief One point on the dream chart, the game's two-axis mood graph: a
 * day's mood, or the mood an entity contributes to it (DreamSys keeps one
 * per previous day and sums them). Read either as the two signed axes or
 * as one halfword to copy the pair at once.
 */
typedef union MoodGraphPoint {
    s16 value; /**< Both axes as one halfword, for copying the point whole. */

    /** @brief The point's two axes, one signed byte each. */
    struct axis {
        s8 dynamic; /**< The chart's static-to-dynamic axis. */
        s8 upper;   /**< The chart's downer-to-upper axis. */
    } axis;         /**< The point read as its two axes. */
} MoodGraphPoint;

#endif
