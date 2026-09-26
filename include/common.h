#ifndef COMMON_H
#define COMMON_H

#include "include_asm.h"
#include "types.h"

/* The byte offset of `member` in `type`: C89's <stddef.h> offsetof, which
 * Sony's stddef.h does not provide. A constant expression to cc1. */
#ifndef offsetof
#define offsetof(type, member) ((unsigned int)&((type *)0)->member)
#endif

typedef union MoodGraphPoint {
    s16 value;

    struct axis {
        s8 dynamic;
        s8 upper;
    } axis;
} MoodGraphPoint;

#endif