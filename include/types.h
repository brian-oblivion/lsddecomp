#ifndef TYPES_H
#define TYPES_H

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

/* Sony's names, under <sys/types.h>'s own guards so that header skips
 * them: game code includes <libgte.h>, <libgpu.h> and <libgs.h> after this
 * file. u_long stays `unsigned int`, not Sony's
 * `unsigned long`: the two are one 32-bit type to cc1, and this way a u32
 * buffer passes to LoadImage() without a cast. */
#ifndef _UCHAR_T
#define _UCHAR_T
typedef unsigned char u_char;
#endif
#ifndef _USHORT_T
#define _USHORT_T
typedef unsigned short u_short;
#endif
#ifndef _UINT_T
#define _UINT_T
typedef unsigned int u_int;
#endif
#ifndef _ULONG_T
#define _ULONG_T
typedef unsigned int u_long;
#endif

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef float f32;
typedef double f64;

typedef int bool;

enum { false, true };

#ifndef NULL
#define NULL (0)
#endif

#endif