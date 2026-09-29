#ifndef TYPES_H
#define TYPES_H

/**
 * @file types.h
 * @brief Integer, float and truth-value types for the R3000 target.
 *
 * On this target `int` and `long` are 32 bits, `long long` is 64, and
 * plain `char` is unsigned (the build passes -funsigned-char). A signed
 * byte is therefore always written `s8`, never `char`.
 *
 * The same types go by three names here: the short names the game code
 * uses (`u8` .. `s64`), the C99 names (`uint8_t` .. `int64_t`), and Sony's
 * `u_char` .. `u_long` from <sys/types.h>.
 */

/* Short names: what the game code is written in. */

/** @brief Unsigned 8-bit integer. */
typedef unsigned char u8;
/** @brief Unsigned 16-bit integer. */
typedef unsigned short u16;
/** @brief Unsigned 32-bit integer. */
typedef unsigned int u32;
/** @brief Unsigned 64-bit integer. */
typedef unsigned long long u64;
/** @brief Signed 8-bit integer. */
typedef signed char s8;
/** @brief Signed 16-bit integer. */
typedef signed short s16;
/** @brief Signed 32-bit integer. */
typedef signed int s32;
/** @brief Signed 64-bit integer. */
typedef signed long long s64;
/** @brief 32-bit float. There is no FPU, so all float math is software. */
typedef float f32;
/** @brief 64-bit float, software like f32. */
typedef double f64;

/* C99 names, for the few places that read better with them. */

/** @brief Unsigned 8-bit integer (C99 spelling). */
typedef u8 uint8_t;
/** @brief Unsigned 16-bit integer (C99 spelling). */
typedef u16 uint16_t;
/** @brief Unsigned 32-bit integer (C99 spelling). */
typedef u32 uint32_t;
/** @brief Unsigned 64-bit integer (C99 spelling). */
typedef u64 uint64_t;
/** @brief Signed 8-bit integer (C99 spelling). */
typedef s8 int8_t;
/** @brief Signed 16-bit integer (C99 spelling). */
typedef s16 int16_t;
/** @brief Signed 32-bit integer (C99 spelling). */
typedef s32 int32_t;
/** @brief Signed 64-bit integer (C99 spelling). */
typedef s64 int64_t;

/* Sony's names. Each sits under the guard <sys/types.h> itself uses, so
 * that header skips it when a unit includes <libgte.h> and friends after
 * this file. u_long is `unsigned int` rather than Sony's `unsigned long`:
 * both are 32 bits here, and this way a u32 buffer passes to LoadImage()
 * and the other u_long * APIs without a cast. */

#ifndef _UCHAR_T
#define _UCHAR_T /**< u_char is defined */
/** @brief Sony's unsigned 8-bit integer. */
typedef u8 u_char;
#endif

#ifndef _USHORT_T
#define _USHORT_T /**< u_short is defined */
/** @brief Sony's unsigned 16-bit integer. */
typedef u16 u_short;
#endif

#ifndef _UINT_T
#define _UINT_T /**< u_int is defined */
/** @brief Sony's unsigned 32-bit integer. */
typedef u32 u_int;
#endif

#ifndef _ULONG_T
#define _ULONG_T /**< u_long is defined */
/** @brief Sony's unsigned 32-bit `long`, the same type as u32 here. */
typedef u32 u_long;
#endif

/** @brief An `int`-sized truth value: zero is false, anything else true. */
typedef int bool;

/** @brief The values of bool. */
enum { false = 0, true = 1 };

#ifndef NULL
#define NULL 0 /**< the null pointer */
#endif

#endif
