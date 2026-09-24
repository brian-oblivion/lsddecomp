/*
 * code_8220_b -- the tail of the BasicClass block, then the game's own
 * per-face polygon renderer. Two unrelated halves, split at func_80018464.
 *
 * 0x80018288..0x80018458 finishes BasicClass, the hand-rolled base class
 * whose framework and 14-slot method table live in code_8220.c and
 * include/code_8220.h: the two list primitives BasicClass's own methods
 * call (FreeBasicClassList, GetNextBasicClass), the array release helper,
 * the vtable accessor, the two notification slots (BasicClass__NotifyParents
 * at +0x030 and BasicClass__OnNotify at +0x038) and the empty +0x034 hook,
 * plus the accessor pair for the pool allocator's re-entrancy flag.
 *
 * 0x80018464..0x8001974C is the renderer. func_80018464 (the unit's one
 * stall, still INCLUDE_ASM) walks a model's face groups, dispatches on each
 * group's tag to one of 13 cases, and per face calls SetupPrimCode, then
 * ProjectTriFace or ProjectQuadFace, then one of Sony's RCpoly* packers via
 * the wrappers in code_8220_c. ProjectTri/QuadFace index the shared vertex
 * array, hand the vertices to the GTE, call TransformAndCullPoly for the
 * transform-and-cull decision, write each vertex's Z into its sort slot, and
 * call back into one of the six StoreSxyPoly** leaves to write the screen XY
 * into the Psy-Q POLY_xx primitive at that primitive type's own offsets.
 * Every GTE access goes through the gte_* macros in include/gte.h; those are
 * Sony's names and are never renamed.
 */

#include "common.h"
#include "code_8220.h"
#include "gte.h"

void FreeBasicClassList(BasicClassListNode **head)
{
    BasicClassListNode *node = *head;

    while (node != NULL) {
        BasicClassListNode *cur = node;
        node = node->next;
        BMemPMgrFree(cur);
    }
}

/* BasicClassMethods slot +0x030. Tell every object holding a reference to
 * `self` that `event` happened, by calling each one's own +0x038 slot with
 * `self` as the sender. BasicClass__Finalize (finalize, code_8220.c) is
 * the only caller in carved C and passes 1. */
void BasicClass__NotifyParents(BasicClass *self, s32 event)
{
    BasicClassListNode *cursor = self->parentRefs;
    BasicClass *parent;

    for (GetNextBasicClass(&parent, &cursor); parent != NULL; GetNextBasicClass(&parent, &cursor)) {
        parent->methods->slot38(parent, self, event);
    }
}

/* BasicClassMethods slot +0x034, and deliberately still a placeholder name.
 * The body is empty, and of the 60 method tables tools/classtable.py finds,
 * 58 have this exact address in slot +0x034; the two that differ
 * (D_8006C0F8, whose header word is a pointer and which is probably a
 * mis-detected table start, and gStyleCueCallbacks, a wholly independent 14-slot
 * class that overrides every slot) are not BasicClass-derived. Nothing in
 * the game overrides it, so nothing establishes what it is for. */
void BasicClass__func_18350(void) {
}

/* BasicClassMethods slot +0x038, the receiving half of NotifyParents:
 * `sender` is telling `self` that `event` happened. The base class treats
 * event 1 as "sender is going away" and drops it from its own children.
 * Subclasses override this and forward to the base first -- Class6B5CC__OnNotify
 * (code_d294) then dispatches on the sender's class tag, func_80065790
 * (code_55dd4) then checks the sender's tag and the same event == 1 -- so
 * `event` is a general notification code, not a boolean. */
void BasicClass__OnNotify(BasicClass *self, void *sender, s32 event)
{
    if (event == 1) {
        self->methods->removeChild(self, (BasicClass *)sender);
    }
}

BasicClassMethods *Get_vtable_BasicClass(void)
{
    return &D_8006B58C;
}

void GetNextBasicClass(BasicClass **outValue, BasicClassListNode **cursor)
{
    if (*cursor != NULL) {
        *outValue = (*cursor)->value;
        *cursor = (*cursor)->next;
    } else {
        *outValue = NULL;
    }
}

void ReleaseBasicClassArray(BasicClass **array, s32 count)
{
    if (count-- > 0) {
        do {
            *array = (BasicClass *)(*array)->methods->release(*array);
            array++;
        } while (count-- > 0);
    }
}

void SetBMemPMgrBusy(s32 val)
{
    gBMemPMgrBusy = val;
}

s32 GetBMemPMgrBusy(void)
{
    return gBMemPMgrBusy;
}

INCLUDE_ASM("asm/nonmatchings/code_8220_b", func_80018464);

/*
 * Finish the GPU command byte of the POLY_xx primitive `prim`, then cache it
 * in the draw context. `prim[3]` is the P_TAG length byte and `prim[7]` is
 * the GPU command byte; func_80018464 writes the (len, code) pair for the
 * primitive type immediately before every one of its 13 calls to this, and
 * all eight pairs it uses are Sony's exactly -- (4, 0x20) POLY_F3, (6, 0x30)
 * POLY_G3, (7, 0x24) POLY_FT3, (5, 0x28) POLY_F4, (9, 0x2C) POLY_FT4,
 * (9, 0x34) POLY_GT3, (8, 0x38) POLY_G4, (12, 0x3C) POLY_GT4.
 *
 * Bit 0x2 of the command byte is the GPU's ABE (semi-transparency) bit and
 * is taken from the context's own flag at +0x1C; bit 0x1 is the shade-texture
 * bit (Psy-Q SetShadeTex) and is taken from the global D_8008E248, which
 * func_80018464 sets from bit 6 of the object's flags word. The two updates
 * are deliberately independent statements -- see this function's match
 * report, factoring them through one local costs the match.
 *
 * The length byte and the finished command byte are then cached at ctx+0x14
 * and ctx+0x15; TransformAndCullPoly re-stamps the length byte onto every
 * primitive it processes from ctx+0x14.
 */
void SetupPrimCode(void *prim, void *ctx)
{
    u8 *a = (u8 *)prim;

    if (*(s32 *)((u8 *)ctx + 0x1C) != 0) {
        a[7] = a[7] | 0x2;
    } else {
        a[7] = a[7] & 0xFD;
    }

    if (D_8008E248 != 0) {
        a[7] = a[7] | 0x1;
    } else {
        a[7] = a[7] & 0xFE;
    }

    *((u8 *)ctx + 0x14) = a[3];
    *((u8 *)ctx + 0x15) = a[7];
}

/*
 * Project one three-vertex face and say whether it survived.
 *
 * `idx0`..`idx2` index the 8-byte vertex array the draw context holds at
 * ctx+0xC; the three resulting pointers are parked in the context's vertex
 * slots at +0xA4/+0xA8/+0xAC and loaded into the GTE. TransformAndCullPoly
 * does the transform and the cull decision. On success each vertex's screen
 * Z goes into the sort slot at +0x14 of the three per-vertex records the
 * context lists at +0x88/+0x8C/+0x90, `storeSxy` writes the screen XY into
 * `prim` at that primitive type's own offsets (one of the StoreSxyPoly**
 * leaves at the bottom of this file), and func_8001A268 computes the screen
 * bounding box over the 3 vertices. Returns 0 drawn, 1 culled.
 *
 * `prim` is only ever handed straight through, so it stays void * here.
 */
s32 ProjectTriFace(void *prim, u8 *ctx, u16 idx0, u16 idx1, u16 idx2, void (*storeSxy)(void *))
{
    *(void **)(ctx + 0xa4) = (u8 *)(*(void **)(ctx + 0xc)) + (s32)idx0 * 8;
    *(void **)(ctx + 0xa8) = (u8 *)(*(void **)(ctx + 0xc)) + (s32)idx1 * 8;
    *(void **)(ctx + 0xac) = (u8 *)(*(void **)(ctx + 0xc)) + (s32)idx2 * 8;

    gte_ldv3(*(void **)(ctx + 0xa4), *(void **)(ctx + 0xa8),
             *(void **)(ctx + 0xac));

    if (TransformAndCullPoly(prim, ctx) != 0) {
        goto fail;
    }

    {
        u8 *p0 = *(u8 **)(ctx + 0x88) + 0x14;
        u8 *p1 = *(u8 **)(ctx + 0x8c) + 0x14;
        u8 *p2 = *(u8 **)(ctx + 0x90) + 0x14;

        gte_stsz3(p0, p1, p2);
    }
    storeSxy(prim);
    func_8001A268(ctx, 3);
    return 0;
fail:
    return 1;
}

/*
 * The four-vertex sibling of ProjectTriFace. The first three vertices go
 * through the same shared transform-and-cull; the fourth is transformed on
 * its own with a single rtps afterwards, which is why `storeSxy` is called
 * twice -- once with 1 to store the first three screen XYs, once with 0 to
 * store the fourth. Sort Zs go to four per-vertex records (+0x94..+0xA0),
 * the fourth vertex's screen XY is also cached at ctx+0x6C, and the bounding
 * box is computed over 4 vertices. Returns 0 drawn, 1 culled.
 */
s32 ProjectQuadFace(void *prim, u8 *ctx, u16 idx0, u16 idx1, u16 idx2, u16 idx3, void (*storeSxy)(void *, s32))
{
    u8 *vtxSlot = ctx + 0xa4;

    *(void **)(ctx + 0xa4) = (u8 *)(*(void **)(ctx + 0xc)) + (s32)idx0 * 8;
    *(void **)(ctx + 0xa8) = (u8 *)(*(void **)(ctx + 0xc)) + (s32)idx1 * 8;
    *(void **)(ctx + 0xac) = (u8 *)(*(void **)(ctx + 0xc)) + (s32)idx2 * 8;
    *(void **)(ctx + 0xb0) = (u8 *)(*(void **)(ctx + 0xc)) + (s32)idx3 * 8;

    gte_ldv3(*(void **)(vtxSlot + 0x0), *(void **)(vtxSlot + 0x4),
             *(void **)(vtxSlot + 0x8));

    if (TransformAndCullPoly(prim, ctx) != 0) {
        goto fail;
    }

    storeSxy(prim, 1);

    gte_ldv0(*(void **)(vtxSlot + 0xc));
    gte_rtps();

    {
        u8 *p0 = *(u8 **)(ctx + 0x94) + 0x14;
        u8 *p1 = *(u8 **)(ctx + 0x98) + 0x14;
        u8 *p2 = *(u8 **)(ctx + 0x9c) + 0x14;
        u8 *p3 = *(u8 **)(ctx + 0xa0) + 0x14;

        gte_stsz4(p0, p1, p2, p3);
    }

    storeSxy(prim, 0);

    gte_stsxy2(ctx + 0x6c);

    func_8001A268(ctx, 4);
    return 0;
fail:
    return 1;
}

/*
 * This unit's local view of the draw context TransformAndCullPoly works on
 * (the `ctx` its callers hand it -- one per-object scratch block, which
 * func_80018464's own caller places in the PS1 scratchpad at 0x1F800000).
 * Only the fields this function touches are typed; the callers still address
 * the rest by offset, and the offsets outside this view that this unit does
 * use are +0x0C the vertex array, +0x1C the semi-transparency flag,
 * +0x14/+0x15 the cached tag length and GPU command byte, +0x88..+0xA0 the
 * per-vertex sort records and +0xA4..+0xB0 the four vertex slots.
 *
 * The three SXY words are deliberately separate fields rather than an array:
 * retail stores them through three independently computed addresses, which
 * is gte_stsxy3()'s three-pointer form.
 */
typedef struct PolyDrawCtx {
    /* +0x000 */ u32 *otBase;        /* ordering table, 4-byte entries */
    /* +0x004 */ s32 otShift;        /* otz >> otShift indexes otBase */
    u8 pad008[0x014 - 0x008];
    /* +0x014 */ u8 tagLen;          /* SetupPrimCode's cached P_TAG length byte */
    u8 pad015[0x020 - 0x015];
    /* +0x020 */ s32 otz;            /* avsz3 result */
    /* +0x024 */ s32 dp;             /* IR0, the depth-cue factor */
    /* +0x028 */ s32 opz;            /* nclip result (MAC0) */
    u8 pad02C[0x030 - 0x02C];
    /* +0x030 */ u32 *otSlot;        /* &otBase[otz >> otShift] */
    u8 pad034[0x05C - 0x034];
    /* +0x05C */ s32 flag;           /* GTE FLAG & 0x40000 */
    /* +0x060 */ s32 sxy0;
    /* +0x064 */ s32 sxy1;
    /* +0x068 */ s32 sxy2;
    u8 pad06C[0x078 - 0x06C];
    /* +0x078 */ s32 saturated;      /* set when the transform saturated */
} PolyDrawCtx;

/*
 * The Psy-Q GPU primitive being filled in. Only the P_TAG length byte is
 * written from here; the GPU command byte one word later (+0x7) is
 * SetupPrimCode's, and the screen XYs are the StoreSxyPoly** leaves'.
 */
typedef struct GpuPrim {
    u8 pad000[3];
    /* +0x003 */ u8 tagLen;          /* P_TAG's length field, little-endian */
} GpuPrim;

/*
 * Perspective-transform the three vertices the caller already loaded into
 * the GTE (gte_ldv3), then cull: bail if the FLAG register shows anything
 * other than clean or SZ3-saturated, if the polygon is back-facing
 * (nclip <= 0), or if the depth-cue factor says it is too close. Otherwise
 * average the Z, cache the screen coordinates and compute the OT bucket.
 * Returns 0 on success, 1 when culled.
 *
 * Splat tagged this "handwritten"; it is ordinary C over the Psy-Q gte_*
 * macros (include/gte.h), and every one of retail's raw words is one of
 * those macros. Earlier rounds carried it as a whole-function __asm__.
 */
s32 TransformAndCullPoly(void *primIn, void *ctxIn)
{
    GpuPrim *prim = primIn;
    PolyDrawCtx *ctx = ctxIn;

    ctx->saturated = 0;
    gte_rtpt();
    prim->tagLen = ctx->tagLen;
    gte_stflg(&ctx->flag);
    if (ctx->flag != 0) {
        if (ctx->flag != 0x40000) {
            return 1;
        }
        ctx->saturated = 1;
    }
    gte_nclip();
    gte_stopz(&ctx->opz);
    if (ctx->opz <= 0) {
        return 1;
    }
    gte_stdp(&ctx->dp);
    if (ctx->dp >= 0x1000) {
        return 1;
    }
    gte_avsz3();
    gte_stotz(&ctx->otz);
    gte_stsxy3(&ctx->sxy0, &ctx->sxy1, &ctx->sxy2);
    ctx->otSlot = &ctx->otBase[ctx->otz >> ctx->otShift];
    return 0;
}

/* The six screen-XY store callbacks ProjectTri/QuadFace invoke, one per
 * Psy-Q primitive type: each writes the GTE's SXY FIFO into that POLY_xx's
 * own vertex offsets. POLY_FT4 and POLY_GT4's equivalents are func_80019774
 * and func_8001979C in code_8220_c. POLY_F3: xy0/xy1/xy2 at +0x8/+0xC/+0x10. */
void StoreSxyPolyF3(void *dst)
{
    gte_stsxy3_f3(dst);
}

/* POLY_G3: +0x8/+0x10/+0x18 (per-vertex RGB between the XYs). */
void StoreSxyPolyG3(void *dst)
{
    gte_stsxy3_g3(dst);
}

/* POLY_FT3: +0x8/+0x10/+0x18 -- byte-identical to the POLY_G3 store, since
 * POLY_G3's per-vertex RGB and POLY_FT3's per-vertex UV are both 4 bytes.
 * Retail keeps them as two separate functions and so does this file; which
 * is which is settled by the (len, code) pair at each one's call site, not
 * by the offsets. */
void StoreSxyPolyFT3(void *dst)
{
    gte_stsxy3_ft3(dst);
}

/* POLY_GT3: +0x8/+0x14/+0x20 (RGB and UV between the XYs). */
void StoreSxyPolyGT3(void *dst)
{
    gte_stsxy3_gt3(dst);
}

/*
 * POLY_F4: xy0/xy1/xy2 at +0x8/+0xC/+0x10, xy3 at +0x14. ProjectQuadFace
 * calls this twice -- storeFirst3 = 1 for the three vertices the shared
 * transform produced, then 0 for the fourth vertex's own rtps result.
 * `p` must be a real unconditionally-computed pointer, not a 0x14(%0)
 * offset inside the asm: see this function's match report.
 */
void StoreSxyPolyF4(void *dst, s32 storeFirst3)
{
    char *p = (char *)dst + 0x14;

    if (storeFirst3) {
        gte_stsxy3_f4(dst);
    } else {
        gte_stsxy2(p);
    }
}

/* POLY_G4: xy0/xy1/xy2 at +0x8/+0x10/+0x18, xy3 at +0x20. Same two-call
 * protocol as StoreSxyPolyF4. */
void StoreSxyPolyG4(void *dst, s32 storeFirst3)
{
    char *p = (char *)dst + 0x20;

    if (storeFirst3) {
        gte_stsxy3_g4(dst);
    } else {
        gte_stsxy2(p);
    }
}
