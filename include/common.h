#ifndef COMMON_H
#define COMMON_H

/**
 * @file common.h
 * @brief Included first by every unit: the integer types (types.h),
 *        small C89 helper macros and the dream chart's point type.
 */

#ifdef PLATFORM_PC
/* The host build has no PS1 assembly: the macros that include it are empty,
 * and the PS1 linker's labels are left out. */
#define INCLUDE_ASM(FOLDER, NAME)
#define INCLUDE_RODATA(FOLDER, NAME)
#else
#include "include_asm.h"
#endif
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

#if !defined(M2CTX) && !defined(PERMUTER) && !defined(PLATFORM_PC)
/** Places a definition in small data, the `.sdata` section: the game's
 * small initialized globals, reached through one base register. Used as
 * `static s32 sName SDATA = 320;`. The definition must have an initializer,
 * and its unit's yaml range must be `.sdata`. */
#define SDATA __attribute__((section(".sdata")))
/** Places a zero-initialized definition in `.sbss`, the uninitialized half of
 * small data: `static s32 sName SBSS = 0;`. The `= 0` is required; without
 * an initializer the section attribute has no effect. */
#define SBSS __attribute__((section(".sbss")))
#else
#define SDATA
#define SBSS
#endif

/** 20.12 fixed point: libgte's ONE is 1 << FIX12_SHIFT, so `n << FIX12_SHIFT`
 * is the integer n as a fixed-point value. */
#define FIX12_SHIFT 12

#ifdef PLATFORM_PC
/** libgte's 32-bit integer, the type of MATRIX.t[] and of VECTOR's members:
 * `int` in psyz's headers. A pointer to one of those members is a
 * `GteLong *`. */
typedef int GteLong;
#else
/** libgte's 32-bit integer, the type of MATRIX.t[] and of VECTOR's members:
 * `long` in Sony's headers. A pointer to one of those members is a
 * `GteLong *`. */
typedef long GteLong;
#endif

/**
 * @brief A position on the dream chart, the two-axis mood graph the game
 * scores each day on. The same type holds a whole day's mood and the
 * nudge one entity gives it; DreamSys keeps one per past day and adds them
 * up. Access the axes separately, or copy both as one halfword.
 */
typedef union MoodGraphPoint {
    /** @brief The two axes, a signed byte each. First, so that an
     * initializer reads `{dynamic, upper}`. */
    struct axis {
        s8 dynamic; /**< The static-to-dynamic axis. */
        s8 upper;   /**< The downer-to-upper axis. */
    } axis;         /**< The point as separate axes. */

    s16 value; /**< Both axes packed together, for whole-point copies. */
} MoodGraphPoint;

#endif
