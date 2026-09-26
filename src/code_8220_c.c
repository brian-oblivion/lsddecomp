/*
 * code_8220_c -- third slice of the code_8220 block (carved round 13): the
 * eight "submit wrappers" code_8220_b's own comment names them (each a
 * tail call to one of Sony's RCpoly* polygon-subdivision packers, or a
 * direct OT splice), the two flag-gated GTE screen-XY store leaves that
 * complete the six-member StoreSxyPoly** family split across this unit and
 * code_8220_b, and the small helpers those wrappers share: a per-vertex
 * pointer-array setup, a screen-space bounding-box/cull update, a shared
 * submit-table header fill, the unaligned vertex xy/uv copy, and the
 * OT-code override setter/getter pair. Every function in the unit is
 * matched; none is `INCLUDE_ASM`. See code_8220_b's own header comment for
 * how ProjectTriFace/ProjectQuadFace and the StoreSxyPoly** leaves there
 * feed into the Submit* wrappers here.
 */

#include "common.h"
#include "code_8220.h"
#include "gte.h"

/* A 2-s16 pair (alignment 2, not 4). Unit-local: six leaves below (four
 * SubmitPoly* wrappers, UpdatePolyBBoxAndCull) each need it to force the
 * unaligned lwl/lwr whole-struct copy retail uses even at offsets that are
 * accidentally 4-aligned -- the compiler only knows the DECLARED alignment
 * of the type, not the runtime address (DECOMPILATION_LEARNINGS, "A struct
 * whose members are all s8/s16 has alignment 2"). Same idiom as
 * FlashbackRotation (include/DreamSys.h), FillRVectors3's PolyXY8/PolyUV4
 * (code_8220.h) and Class866E8__SetTargetAndBuildRates. Formerly five separately typedef'd
 * copies (Vec2s16_98/_C04/_EE4/_A64/_268, one per call site, each named
 * for its own file offset); merged into one unit-local type round 77
 * (alpha) -- a typedef's spelling never affects codegen, only its layout,
 * and all five were byte-identical `{ s16 x, y; }`. */
typedef struct {
    s16 x, y;
} Vec2s16;

void StoreSxyPolyFT4(void *dst, s32 storeFirst3) {
    if (storeFirst3) {
        gte_stsxy3_ft4(dst);
    } else {
        char *p = (char *)dst + 0x20;

        gte_stsxy2(p);
    }
}

void StoreSxyPolyGT4(void *dst, s32 storeFirst3) {
    if (storeFirst3) {
        gte_stsxy3_gt4(dst);
    } else {
        char *p = (char *)dst + 0x2c;

        gte_stsxy2(p);
    }
}

/* Returns the next packet pointer: prim + sizeof(POLY_F3) = 0x14 when the
 * primitive is spliced into the OT directly, else RCpolyF3's own return.
 * RCpolyF3 is declared void in code_8220.h (its return type is track 2's to
 * settle), hence the cast. */
void *SubmitPolyF3(void *prim, void *ctx) {
    if (*(s32 *)((u8 *)ctx + 0x78) != 0) {
        FillDivPolygonHeader(gDivPolygon3, ctx, (u8 *)prim + 0x4, 0, 0, 0);
        FillRVectors3((PolyVtx **)((u8 *)ctx + 0x88), (PolyVtx **)((u8 *)ctx + 0xA4),
                      (PolyUV4 *)((u8 *)prim + 0x8), (PolyUV4 *)((u8 *)prim + 0xC),
                      (PolyUV4 *)((u8 *)prim + 0x10));
        return ((void *(*)(void *, void *))RCpolyF3)(prim, gDivPolygon3);
    }
    ((OtTag *)prim)->addr = (*(OtTag **)((u8 *)ctx + 0x30))->addr;
    (*(OtTag **)((u8 *)ctx + 0x30))->addr = (u32)prim;
    return (u8 *)prim + 0x14;
}

/* Returns the next packet pointer: prim + sizeof(POLY_G3) = 0x1C when the
 * primitive is spliced into the OT directly, else RCpolyG3's own return
 * (cast: RCpolyG3 is declared void in code_8220.h). Same shape as
 * SubmitPolyF3. */
void *SubmitPolyG3(void *prim, void *ctx) {
    u8 *self = (u8 *)prim;
    u8 *c = (u8 *)ctx;

    if (*(s32 *)(c + 0x78) != 0) {
        FillDivPolygonHeader(gDivPolygon3, c, self + 0x4, 0, 0, 0);
        FillRVectors3((PolyVtx **)(c + 0x88), (PolyVtx **)(c + 0xA4), (PolyUV4 *)(self + 0x8),
                      (PolyUV4 *)(self + 0x10), (PolyUV4 *)(self + 0x18));

        *(u16 *)(*(u8 **)(c + 0x88) + 0xA) = *(u8 *)(self + 0xF);
        *(u16 *)(*(u8 **)(c + 0x8C) + 0xA) = *(u8 *)(self + 0xF);
        *(u16 *)(*(u8 **)(c + 0x90) + 0xA) = *(u8 *)(self + 0x17);

        *(Vec2s16 *)(*(u8 **)(c + 0x88) + 0xC) = *(Vec2s16 *)(self + 0x4);
        *(Vec2s16 *)(*(u8 **)(c + 0x8C) + 0xC) = *(Vec2s16 *)(self + 0xC);
        *(Vec2s16 *)(*(u8 **)(c + 0x90) + 0xC) = *(Vec2s16 *)(self + 0x14);

        return ((void *(*)(void *, void *))RCpolyG3)(self, gDivPolygon3);
    }
    ((OtTag *)self)->addr = (*(OtTag **)(c + 0x30))->addr;
    (*(OtTag **)(c + 0x30))->addr = (u32)self;
    return self + 0x1C;
}

/* Returns the next packet pointer: prim + sizeof(POLY_FT3) = 0x20 when the
 * primitive is spliced into the OT directly, else RCpolyFT3's own return
 * (cast: RCpolyFT3 is declared void in code_8220.h). Same shape as
 * SubmitPolyF3. */
void *SubmitPolyFT3(void *prim, void *ctx) {
    if (*(s32 *)((u8 *)ctx + 0x78) != 0) {
        FillDivPolygonHeader(gDivPolygon3, ctx, (u8 *)prim + 0x4, 1, *(u16 *)((u8 *)prim + 0xE),
                             *(u16 *)((u8 *)prim + 0x16));
        FillRVectors3((PolyVtx **)((u8 *)ctx + 0x88), (PolyVtx **)((u8 *)ctx + 0xA4),
                      (PolyUV4 *)((u8 *)prim + 0x8), (PolyUV4 *)((u8 *)prim + 0x10),
                      (PolyUV4 *)((u8 *)prim + 0x18));

        *(u16 *)(*(u8 **)((u8 *)ctx + 0x88) + 0xA) = *(u16 *)((u8 *)prim + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)ctx + 0x8C) + 0xA) = *(u16 *)((u8 *)prim + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)ctx + 0x90) + 0xA) = *(u16 *)((u8 *)prim + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)ctx + 0x88) + 0x8) = *(u16 *)((u8 *)prim + 0xC);
        *(u16 *)(*(u8 **)((u8 *)ctx + 0x8C) + 0x8) = *(u16 *)((u8 *)prim + 0x14);
        *(u16 *)(*(u8 **)((u8 *)ctx + 0x90) + 0x8) = *(u16 *)((u8 *)prim + 0x1C);

        return ((void *(*)(void *, void *))RCpolyFT3)(prim, gDivPolygon3);
    }
    ((OtTag *)prim)->addr = (*(OtTag **)((u8 *)ctx + 0x30))->addr;
    (*(OtTag **)((u8 *)ctx + 0x30))->addr = (u32)prim;
    return (u8 *)prim + 0x20;
}

/* Returns the next packet pointer: prim + sizeof(POLY_F4) = 0x18 when the
 * primitive is spliced into the OT directly, else RCpolyF4's own return
 * (cast: RCpolyF4 is declared void in code_8220.h). Same shape as
 * SubmitPolyF3. */
void *SubmitPolyF4(void *prim, void *ctx) {
    if (*(s32 *)((u8 *)ctx + 0x78) != 0) {
        FillDivPolygonHeader(gDivPolygon4, ctx, (u8 *)prim + 0x4, 0, 0, 0);
        FillRVectors4((u8 *)ctx + 0x94, (u8 *)ctx + 0xA4, (u8 *)prim + 0x8, (u8 *)prim + 0xC,
                      (u8 *)prim + 0x10, (u8 *)prim + 0x14);
        return ((void *(*)(void *, void *))RCpolyF4)(prim, gDivPolygon4);
    }
    ((OtTag *)prim)->addr = (*(OtTag **)((u8 *)ctx + 0x30))->addr;
    (*(OtTag **)((u8 *)ctx + 0x30))->addr = (u32)prim;
    return (u8 *)prim + 0x18;
}

/* Returns the next packet pointer: prim + sizeof(POLY_G4) = 0x24 when the
 * primitive is spliced into the OT directly, else RCpolyG4's own return
 * (cast: RCpolyG4 is declared void in code_8220.h). Same shape as
 * SubmitPolyF3. */
void *SubmitPolyG4(void *prim, void *ctx) {
    u8 *self = (u8 *)prim;
    u8 *c = (u8 *)ctx;

    if (*(s32 *)(c + 0x78) != 0) {
        FillDivPolygonHeader(gDivPolygon4, c, self + 0x4, 0, 0, 0);
        FillRVectors4(c + 0x94, c + 0xA4, self + 0x8, self + 0x10, self + 0x18, self + 0x20);

        *(u16 *)(*(u8 **)(c + 0x94) + 0xA) = *(u8 *)(self + 0xF);
        *(u16 *)(*(u8 **)(c + 0x98) + 0xA) = *(u8 *)(self + 0xF);
        *(u16 *)(*(u8 **)(c + 0x9C) + 0xA) = *(u8 *)(self + 0x17);
        *(u16 *)(*(u8 **)(c + 0xA0) + 0xA) = *(u8 *)(self + 0x1F);

        *(Vec2s16 *)(*(u8 **)(c + 0x94) + 0xC) = *(Vec2s16 *)(self + 0x4);
        *(Vec2s16 *)(*(u8 **)(c + 0x98) + 0xC) = *(Vec2s16 *)(self + 0xC);
        *(Vec2s16 *)(*(u8 **)(c + 0x9C) + 0xC) = *(Vec2s16 *)(self + 0x14);
        *(Vec2s16 *)(*(u8 **)(c + 0xA0) + 0xC) = *(Vec2s16 *)(self + 0x1C);

        return ((void *(*)(void *, void *))RCpolyG4)(prim, gDivPolygon4);
    }
    ((OtTag *)self)->addr = (*(OtTag **)(c + 0x30))->addr;
    (*(OtTag **)(c + 0x30))->addr = (u32)self;
    return (u8 *)prim + 0x24;
}

/* Returns the next packet pointer: prim + sizeof(POLY_FT4) = 0x28 when the
 * primitive is spliced into the OT directly, else RCpolyFT4's own return
 * (cast: RCpolyFT4 is declared void in code_8220.h). Same shape as
 * SubmitPolyF3. */
void *SubmitPolyFT4(void *prim, void *ctx) {
    if (*(s32 *)((u8 *)ctx + 0x78) != 0) {
        FillDivPolygonHeader(gDivPolygon4, ctx, (u8 *)prim + 0x4, 1, *(u16 *)((u8 *)prim + 0xE),
                             *(u16 *)((u8 *)prim + 0x16));
        FillRVectors4((u8 *)ctx + 0x94, (u8 *)ctx + 0xA4, (u8 *)prim + 0x8, (u8 *)prim + 0x10,
                      (u8 *)prim + 0x18, (u8 *)prim + 0x20);

        *(u16 *)(*(u8 **)((u8 *)ctx + 0x94) + 0xA) = *(u16 *)((u8 *)prim + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)ctx + 0x98) + 0xA) = *(u16 *)((u8 *)prim + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)ctx + 0x9C) + 0xA) = *(u16 *)((u8 *)prim + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)ctx + 0xA0) + 0xA) = *(u16 *)((u8 *)prim + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)ctx + 0x94) + 0x8) = *(u16 *)((u8 *)prim + 0xC);
        *(u16 *)(*(u8 **)((u8 *)ctx + 0x98) + 0x8) = *(u16 *)((u8 *)prim + 0x14);
        *(u16 *)(*(u8 **)((u8 *)ctx + 0x9C) + 0x8) = *(u16 *)((u8 *)prim + 0x1C);
        *(u16 *)(*(u8 **)((u8 *)ctx + 0xA0) + 0x8) = *(u16 *)((u8 *)prim + 0x24);

        return ((void *(*)(void *, void *))RCpolyFT4)(prim, gDivPolygon4);
    }
    ((OtTag *)prim)->addr = (*(OtTag **)((u8 *)ctx + 0x30))->addr;
    (*(OtTag **)((u8 *)ctx + 0x30))->addr = (u32)prim;
    return (u8 *)prim + 0x28;
}

/* Returns the next packet pointer: prim + sizeof(POLY_GT3) = 0x28 when the
 * primitive is spliced into the OT directly, else RCpolyGT3's own return
 * (cast: RCpolyGT3 is declared void in code_8220.h). Same shape as
 * SubmitPolyF3. */
void *SubmitPolyGT3(void *prim, void *ctx) {
    u8 *self = (u8 *)prim;
    u8 *c = (u8 *)ctx;

    if (*(s32 *)(c + 0x78) != 0) {
        FillDivPolygonHeader(gDivPolygon3, c, self + 0x4, 1, *(u16 *)(self + 0xE), *(u16 *)(self + 0x1A));
        FillRVectors3((PolyVtx **)(c + 0x88), (PolyVtx **)(c + 0xA4), (PolyUV4 *)(self + 0x8),
                      (PolyUV4 *)(self + 0x14), (PolyUV4 *)(self + 0x20));

        *(u16 *)(*(u8 **)(c + 0x88) + 0xA) = *(u16 *)(self + 0x26);
        *(u16 *)(*(u8 **)(c + 0x8C) + 0xA) = *(u16 *)(self + 0x26);
        *(u16 *)(*(u8 **)(c + 0x90) + 0xA) = *(u16 *)(self + 0x26);

        *(Vec2s16 *)(*(u8 **)(c + 0x88) + 0xC) = *(Vec2s16 *)(self + 0x4);
        *(Vec2s16 *)(*(u8 **)(c + 0x8C) + 0xC) = *(Vec2s16 *)(self + 0x10);
        *(Vec2s16 *)(*(u8 **)(c + 0x90) + 0xC) = *(Vec2s16 *)(self + 0x1C);

        *(u16 *)(*(u8 **)(c + 0x88) + 0x8) = *(u16 *)(self + 0xC);
        *(u16 *)(*(u8 **)(c + 0x8C) + 0x8) = *(u16 *)(self + 0x18);
        *(u16 *)(*(u8 **)(c + 0x90) + 0x8) = *(u16 *)(self + 0x24);

        return ((void *(*)(void *, void *))RCpolyGT3)(prim, gDivPolygon3);
    }
    ((OtTag *)self)->addr = (*(OtTag **)(c + 0x30))->addr;
    (*(OtTag **)(c + 0x30))->addr = (u32)self;
    return (u8 *)prim + 0x28;
}

/* Returns the next packet pointer: prim + sizeof(POLY_GT4) = 0x34 when the
 * primitive is spliced into the OT directly, else RCpolyGT4's own return
 * (cast: RCpolyGT4 is declared void in code_8220.h). Same shape as
 * SubmitPolyF3. */
void *SubmitPolyGT4(void *prim, void *ctx) {
    u8 *self = (u8 *)prim;
    u8 *c = (u8 *)ctx;

    if (*(s32 *)(c + 0x78) != 0) {
        FillDivPolygonHeader(gDivPolygon4, c, self + 0x4, 1, *(u16 *)(self + 0xE), *(u16 *)(self + 0x1A));
        FillRVectors4(c + 0x94, c + 0xA4, self + 0x8, self + 0x14, self + 0x20, self + 0x2C);

        *(u16 *)(*(u8 **)(c + 0x94) + 0xA) = *(u16 *)(self + 0x26);
        *(u16 *)(*(u8 **)(c + 0x98) + 0xA) = *(u16 *)(self + 0x26);
        *(u16 *)(*(u8 **)(c + 0x9C) + 0xA) = *(u16 *)(self + 0x32);
        *(u16 *)(*(u8 **)(c + 0xA0) + 0xA) = *(u16 *)(self + 0x32);

        *(Vec2s16 *)(*(u8 **)(c + 0x94) + 0xC) = *(Vec2s16 *)(self + 0x4);
        *(Vec2s16 *)(*(u8 **)(c + 0x98) + 0xC) = *(Vec2s16 *)(self + 0x10);
        *(Vec2s16 *)(*(u8 **)(c + 0x9C) + 0xC) = *(Vec2s16 *)(self + 0x1C);
        *(Vec2s16 *)(*(u8 **)(c + 0xA0) + 0xC) = *(Vec2s16 *)(self + 0x28);

        *(u16 *)(*(u8 **)(c + 0x94) + 0x8) = *(u16 *)(self + 0xC);
        *(u16 *)(*(u8 **)(c + 0x98) + 0x8) = *(u16 *)(self + 0x18);
        *(u16 *)(*(u8 **)(c + 0x9C) + 0x8) = *(u16 *)(self + 0x24);
        *(u16 *)(*(u8 **)(c + 0xA0) + 0x8) = *(u16 *)(self + 0x30);

        return ((void *(*)(void *, void *))RCpolyGT4)(prim, gDivPolygon4);
    }
    ((OtTag *)self)->addr = (*(OtTag **)(c + 0x30))->addr;
    (*(OtTag **)(c + 0x30))->addr = (u32)self;
    return (u8 *)prim + 0x34;
}

/* Called twice, once per arity, from the render-context setup in
 * code_8220_b (`InitDivPolygonPtrs(ctx + 0x88, gDivPolygon3, 3)`,
 * `InitDivPolygonPtrs(ctx + 0x94, gDivPolygon4, 4)`): writes a
 * running pointer through `table`'s own per-vertex records (stride 0x18,
 * starting at `table + 0x18`) into two parallel arrays -- `dst[i]` (the
 * context's own vertex-record pointer slots that the SubmitPoly* wrappers'
 * `ctx+0x88`/`ctx+0x94` read) and `table`'s own mirror array at `+0xA8`
 * (tri) or `+0xF0` (quad). `kind` doubles as the loop count (3 or 4),
 * matching ProjectTriFace/ProjectQuadFace's own arity code. Mechanics
 * established; WHY the table also keeps its own copy of the same pointers
 * is not (round 77, tier B). */
void InitDivPolygonPtrs(void *dst, void *table, s32 kind) {
    u8 *src = (u8 *)table + 0x18;
    u8 *dst0 = (u8 *)dst;
    u8 *dst1 = (kind == 4) ? (u8 *)table + 0xF0 : (u8 *)table + 0xA8;

    while (kind-- > 0) {
        *(void **)dst1 = src;
        *(void **)dst0 = src;
        src += 0x18;
        dst1 += 4;
        dst0 += 4;
    }
}

/* Leaf with an unused 0x20 frame (the Vec2s16 copies' temporary). Retail's
 * `addiu $sp,$sp,-0x20` sits in the loop-skip branch's delay slot because
 * yp is set only inside the guard: nothing else before the branch is
 * movable, so reorg takes the prologue's sp adjust (round 75). `ctx` is the
 * same per-face draw context ProjectTriFace/ProjectQuadFace and the
 * Submit* wrappers use (its SXY0-2 cache at +0x60/+0x64/+0x68 and its
 * culled flag at +0x78 are exactly the fields this function reads and
 * sets; see the extern comment in code_8220.h). Tracks the running 2D
 * screen bounding box of `count` XY samples starting at ctx+0x64 into
 * ctx+0x70/0x72/0x74/0x76 (min x/y, max x/y) and sets the cull flag when
 * either span reaches 0x101 -- i.e. when the primitive's screen extent in
 * either axis would exceed what a single draw primitive can represent. */
void UpdatePolyBBoxAndCull(void *ctx, s32 count) {
    u8 *self = (u8 *)ctx;
    s16 *xp, *yp, *end;

    *(Vec2s16 *)(self + 0x74) = *(Vec2s16 *)(self + 0x60);
    *(Vec2s16 *)(self + 0x70) = *(Vec2s16 *)(self + 0x74);

    xp = (s16 *)(self + 0x64);
    end = (s16 *)(self + (count << 2) + 0x5C);

    if (xp < end) {
        yp = (s16 *)(self + 0x66);
        do {
            if (*xp < *(s16 *)(self + 0x70)) {
                *(s16 *)(self + 0x70) = *xp;
            }
            if (*yp < *(s16 *)(self + 0x72)) {
                *(s16 *)(self + 0x72) = *yp;
            }
            if (*(s16 *)(self + 0x74) < *xp) {
                *(s16 *)(self + 0x74) = *xp;
            }
            if (*(s16 *)(self + 0x76) < *yp) {
                *(s16 *)(self + 0x76) = *yp;
            }
            xp += 2;
            yp += 2;
        } while (xp < end);
    }

    if (*(s16 *)(self + 0x74) - *(s16 *)(self + 0x70) >= 0x101) {
        *(s32 *)(self + 0x78) = 1;
    }
    if (*(s16 *)(self + 0x76) - *(s16 *)(self + 0x72) >= 0x101) {
        *(s32 *)(self + 0x78) = 1;
    }
}

/* sDivClipWidth/sDivClipHeight -- read only by FillDivPolygonHeader, unconditionally
 * copied into every submit table's +0x4/+0x8 words. No second accessor
 * establishes what either holds beyond "a GPU header word"; left as `D_`
 * names (round 77, insufficient evidence for gPoly*-style names). */
extern s32 sDivClipWidth;
extern s32 sDivClipHeight;

/* dc_cb+0x4 (Sony; config/psyq-objects.ld pins `dc_cb` at 0x80090c14).
 * rename.py refuses a game name here -- the only rename it would allow is
 * to `dc_cb` itself, and this unit only ever reads the +0x4 word, so it
 * keeps the D_ spelling. Written by code_8220_b from a render object's
 * flags bits 9-11; read here as the default OT/code word when no override
 * is set (see sNdivOverrideSet below). */
extern s32 D_80090C18;

extern s32 sNdivOverrideSet;
extern s32 sNdivOverride;

/* Populates a submit table's (`table`, one of gDivPolygon3/
 * gDivPolygon4) common header fields ahead of a Submit* wrapper's
 * RCpoly* call: +0x0 an OT/code word (sNdivOverride when
 * sNdivOverrideSet, else the D_80090C18 default), +0x4 sDivClipWidth,
 * +0x8 sDivClipHeight -- these three are UNCONDITIONAL; only the two u16 args
 * at +0xC/+0xE are gated on `hasUv1Codes`. +0x10 is an unaligned PolyUV4
 * copied from `*uv` (same lwl/lwr idiom as FillRVectors3); +0x14 is the
 * plain word at `ctx + 0x30` (the caller's computed OT bucket pointer,
 * code_8220.h). Pure header-populate leaf, same shape at all 8 call
 * sites: tier A. */
void FillDivPolygonHeader(void *table, void *ctx, PolyUV4 *uv, s32 hasUv1Codes, u16 uv1Clut,
                          u16 uv1TPage) {
    u8 *dst = (u8 *)table;
    u8 *c = (u8 *)ctx;
    s32 val;
    s32 code;
    s32 code2;

    if (sNdivOverrideSet) {
        val = sNdivOverride;
    } else {
        val = D_80090C18;
    }
    code = sDivClipWidth;
    code2 = sDivClipHeight;

    *(s32 *)dst = val;
    *(s32 *)(dst + 0x4) = code;
    *(s32 *)(dst + 0x8) = code2;

    if (hasUv1Codes != 0) {
        *(u16 *)(dst + 0xC) = uv1Clut;
        *(u16 *)(dst + 0xE) = uv1TPage;
    }

    *(PolyUV4 *)(dst + 0x10) = *uv;
    *(s32 *)(dst + 0x14) = *(s32 *)(c + 0x30);
}

/*
 * Copies three unaligned 8-byte fields (src[i]->xy -> dst[i]->xy) and three
 * unaligned 4-byte fields (*uv0/*uv1/*uv2 -> dst[i]->uv). Retail does each
 * with lwl/lwr + swl/swr, and the idiom that reproduces that is the struct
 * types themselves: PolyXY8 and PolyUV4 are ALL-s16, so their alignment is
 * 2, and a whole-struct assignment of an alignment-2 type is what GCC 2.6.3
 * emits as the unaligned pair. One stray s32 member and the copy becomes
 * aligned lw/sw and stops matching (DECOMPILATION_LEARNINGS, "A struct
 * whose members are all s8/s16 has alignment 2"). Round 13 first matched
 * this as a whole-function __asm__ transcription; the head reworked it into
 * these six assignments, byte-exact, and CLAUDE.md HARD RULE 6 cites it as
 * the example of "hard to type" not being "no C form".
 */
void FillRVectors3(PolyVtx **dst, PolyVtx **src, PolyUV4 *uv0, PolyUV4 *uv1, PolyUV4 *uv2) {
    dst[0]->xy = src[0]->xy;
    dst[1]->xy = src[1]->xy;
    dst[2]->xy = src[2]->xy;
    dst[0]->uv = *uv0;
    dst[1]->uv = *uv1;
    dst[2]->uv = *uv2;
}

void FillRVectors4(PolyVtx **dst, PolyVtx **src, PolyUV4 *uv0, PolyUV4 *uv1, PolyUV4 *uv2, PolyUV4 *uv3) {
    FillRVectors3(dst, src, uv0, uv1, uv2);
    dst[3]->xy = src[3]->xy;
    dst[3]->uv = *uv3;
}

extern s32 sNdivOverrideSet;
extern s32 sNdivOverride;

/* Setter matching FillDivPolygonHeader's read side: `enable` gates whether
 * FillDivPolygonHeader's header word 0 comes from `code` (this call's second
 * argument, stored only when `enable` is set) or from the per-object
 * D_80090C18 default. Tier A. */
void SetNdivOverride(s32 enable, s32 code) {
    sNdivOverrideSet = enable;
    if (enable) {
        sNdivOverride = code;
    }
}
