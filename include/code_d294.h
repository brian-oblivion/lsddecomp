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

/* Segment-against-box clipping for the link tests. CalcBoxOutcode returns
 * a point's 6-bit outcode against a box (x 8/4, y 2/1, z 0x20/0x10; high bit
 * past max, low bit before min), unmasked. ClipSegmentToBox returns 0 when
 * p1..p2 misses the box, 1 when both ends are inside, 2 or 3 when only p1 or
 * only p2 is, writing the boundary crossing to `out` when non-NULL.
 * BisectSegmentToBox finds that crossing by halving from the inside point
 * `near` towards the outside point `far`. */
void BisectSegmentToBox(TmdVec3 *out, TmdBox *box, TmdVec3 *near, TmdVec3 *far);
extern s32 CalcBoxOutcode(TmdBox *box, TmdVec3 *point);
s32 ClipSegmentToBox(TmdVec3 *out, TmdBox *box, TmdVec3 *p1, TmdVec3 *p2);

#endif
