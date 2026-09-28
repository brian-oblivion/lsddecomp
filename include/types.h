#ifndef TYPES_H
#define TYPES_H

/**
 * @file types.h
 * @brief The fixed-width integer, float and boolean types every unit uses:
 *        the project's `s8`..`u64`, the C99 `int8_t`..`uint64_t` names, and
 *        Sony's `u_char`..`u_long`.
 *
 * The target is 32-bit little-endian MIPS: `int` and `long` are both 32
 * bits, `long long` is 64, and plain `char` is unsigned, so a signed byte is
 * always spelled `s8` (or `int8_t`), never `char`.
 */

/** @brief Signed 8-bit integer (C99 name; plain `char` is unsigned here). */
typedef signed char int8_t;
/** @brief Signed 16-bit integer (C99 name). */
typedef short int16_t;
/** @brief Signed 32-bit integer (C99 name). */
typedef int int32_t;
/** @brief Signed 64-bit integer (C99 name). */
typedef long long int64_t;
/** @brief Unsigned 8-bit integer (C99 name). */
typedef unsigned char uint8_t;
/** @brief Unsigned 16-bit integer (C99 name). */
typedef unsigned short uint16_t;
/** @brief Unsigned 32-bit integer (C99 name). */
typedef unsigned int uint32_t;
/** @brief Unsigned 64-bit integer (C99 name). */
typedef unsigned long long uint64_t;

/* Sony's names, under <sys/types.h>'s own guards so that header skips
 * them: game code includes <libgte.h>, <libgpu.h> and <libgs.h> after this
 * file. u_long stays `unsigned int`, not Sony's `unsigned long`: the two
 * are one 32-bit type on this target, and this way a u32 buffer passes to
 * LoadImage() without a cast. */
#ifndef _UCHAR_T
#define _UCHAR_T
/** @brief Sony's unsigned 8-bit integer (<sys/types.h>). */
typedef unsigned char u_char;
#endif
#ifndef _USHORT_T
#define _USHORT_T
/** @brief Sony's unsigned 16-bit integer (<sys/types.h>). */
typedef unsigned short u_short;
#endif
#ifndef _UINT_T
#define _UINT_T
/** @brief Sony's unsigned 32-bit integer (<sys/types.h>). */
typedef unsigned int u_int;
#endif
#ifndef _ULONG_T
#define _ULONG_T
/** @brief Sony's unsigned 32-bit `long`, spelled `unsigned int` so it is
 * the same type as u32. */
typedef unsigned int u_long;
#endif

/** @brief Signed 8-bit integer. */
typedef signed char s8;
/** @brief Signed 16-bit integer. */
typedef signed short s16;
/** @brief Signed 32-bit integer. */
typedef signed int s32;
/** @brief Signed 64-bit integer. */
typedef signed long long s64;
/** @brief Unsigned 8-bit integer. */
typedef unsigned char u8;
/** @brief Unsigned 16-bit integer. */
typedef unsigned short u16;
/** @brief Unsigned 32-bit integer. */
typedef unsigned int u32;
/** @brief Unsigned 64-bit integer. */
typedef unsigned long long u64;
/** @brief 32-bit float (software floating point: the CPU has no FPU). */
typedef float f32;
/** @brief 64-bit float (software floating point). */
typedef double f64;

/** @brief A truth value, `int`-sized: 0 is false, anything else true. */
typedef int bool;

/** @brief The two values of bool. */
enum { false, true };

#ifndef NULL
#define NULL (0)
#endif

#endif
