#ifndef CODE_D294_H
#define CODE_D294_H

#include "common.h"
#include "SceneNode.h"

/* The private header of SceneNode's three method units, code_d294.c,
 * code_d294_b.c and code_d294_c.c. The class itself (object, method table,
 * method prototypes) is include/SceneNode.h, and the model's types (TmdVec3,
 * TmdBox, TmdHull) are include/TmdModel.h. This holds the free helpers those
 * units define or call by symbol: the rotation and scale inputs, the
 * matrix-over-array transforms, the attribute-word accessor and the
 * box-clipping primitives. Sony's functions (RotMatrix, MulMatrix2,
 * ApplyMatrixLV, ratan2, GsInitCoordinate2, GsLinkObject4) come from
 * <libgte.h> and <libgs.h>, which each unit includes before this. */

extern void *BMemPMgrAlloc(s32 size);
extern void BMemPMgrFree(void *arg);

/* A Ratio16 (num / den) as 20.12 fixed point. updateRotation and updateScale
 * apply it to each of their three entries. */
extern s32 RatioToFixed12(void *pair);

/* The identity inputs SceneNode__Reset hands to updateRotation and
 * updateScale: three Ratio16s each, {0/1, 0/1, 0/1} and {1/1, 1/1, 1/1}. */
extern u8 ROTATION_ZERO[0xC];
extern u8 SCALE_ONE[0xC];

/* dst[i] = m * src[i] over `count` elements, through Sony's ApplyMatrixSV
 * (6-byte s16 vectors) and ApplyMatrixLV (0xC-byte s32 vectors). The first
 * argument is the destination; dst == src transforms in place. */
extern void ApplyMatrixToSVArray(TmdVec3 *dst, TmdVec3 *src, s32 count, MATRIX *m);
void ApplyMatrixToLVArray(void *dst, void *src, s32 count, void *m);

/* Replaces the `width` bits at bit `shift` of *word with `value` and returns
 * the field's old contents. The attribute setters (SceneNode__SetDisplay and
 * the rest) are wrappers around it over GsDOBJ2.attribute. */
extern u32 GetSetBitField(u32 *word, s32 shift, s32 width, u32 value);

/* Bit positions in GsDOBJ2.attribute (include/psyq/libgs.h), the fields the
 * SceneNode attribute setters (code_d294.c, code_d294_b.c) replace. */
#define ATTR_LDIM_SHIFT 0      /* GsLDIM0..GsLDIM7, 3 bits */
#define ATTR_LIGHTMODE_SHIFT 3 /* GsFOG|GsMATE|GsLLMOD, 3 bits */
#define ATTR_LOFF_SHIFT 6      /* GsLOFF */
#define ATTR_ZIGNR_SHIFT 7     /* GsZIGNR */
#define ATTR_NBACKC_SHIFT 8    /* GsNBACKC */
#define ATTR_DIV_SHIFT 9       /* GsDIV1..GsDIV5, 3 bits */
#define ATTR_ABR_SHIFT 28      /* GsAZERO..GsATHREE, 2 bits */
#define ATTR_ALON_SHIFT 30     /* GsALON */
#define ATTR_DOFF_SHIFT 31     /* GsDOFF */

/* Segment-against-box clipping for the link tests. CalcBoxOutcode returns
 * a point's 6-bit outcode against a box (x 8/4, y 2/1, z 0x20/0x10; high bit
 * past max, low bit before min), unmasked. ClipSegmentToBox returns 0 when
 * p1..p2 misses the box, 1 when both ends are inside, 2 or 3 when only p1 or
 * only p2 is, writing the boundary crossing to `out` when non-NULL.
 * BisectSegmentToBox finds that crossing by halving from the inside point
 * `near` towards the outside point `far`. */
enum ClipResult {
    CLIP_MISS = 0,      /* the segment misses the box */
    CLIP_INSIDE = 1,    /* both ends are inside */
    CLIP_P1_INSIDE = 2, /* only p1 is inside */
    CLIP_P2_INSIDE = 3  /* only p2 is inside */
};

/* CalcBoxOutcode's bits: per axis, MAX when the point is past the box's
 * maximum and MIN when it is before its minimum. */
#define OUTCODE_Y_MIN 0x01
#define OUTCODE_Y_MAX 0x02
#define OUTCODE_X_MIN 0x04
#define OUTCODE_X_MAX 0x08
#define OUTCODE_Z_MIN 0x10
#define OUTCODE_Z_MAX 0x20

void BisectSegmentToBox(TmdVec3 *out, TmdBox *box, TmdVec3 *near, TmdVec3 *far);
extern s32 CalcBoxOutcode(TmdBox *box, TmdVec3 *point);
s32 ClipSegmentToBox(TmdVec3 *out, TmdBox *box, TmdVec3 *p1, TmdVec3 *p2);

#endif
