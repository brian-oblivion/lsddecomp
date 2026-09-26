/*
 * code_8220_c -- the back end of the game's per-face polygon renderer
 * (code_8220_b walks the faces and projects them): putting one finished
 * GPU primitive into the ordering table.
 *
 * Each SubmitPoly* wrapper takes one of libgpu's POLY_F3 ... POLY_GT4
 * packets that ProjectTriFace/ProjectQuadFace have filled, and the per-object
 * draw context. A face whose screen box fits links straight into its OT
 * slot with addPrim. A face flagged for division (too large on screen, see
 * FlagLargePolyForDivide; or with a saturated screen Z, see code_8220_b's
 * TransformAndCullPoly) is instead copied into one of the two DIVPOLYGON
 * work buffers and handed to Sony's RCpoly* packer, which subdivides it
 * into smaller primitives.
 * Either way the wrapper returns the next free packet-buffer address.
 *
 * Also here: the last two StoreSxyPoly** GTE store leaves (the other four
 * are in code_8220_b), and the helpers that fill a DIVPOLYGON's header and
 * its RVECTOR vertex records, plus the ndiv override setter.
 */

#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include "code_8220.h"
#include "gte.h"

/* The widest or tallest screen extent, in pixels, a face may have and still
 * be linked into the OT as one primitive; FlagLargePolyForDivide sends
 * anything larger through Sony's RCpoly* subdivision. 256 is also the width
 * of one texture page. */
#define MAX_UNDIVIDED_SPAN 256

/* The per-object draw context, as this unit sees it: code_8220_b's
 * func_80018464 fills it for each object (in the scratchpad) and hands it to
 * every per-face call. Only the fields read or written here are named;
 * code_8220_b keeps its own view of the rest. */
typedef struct PolyDrawCtx {
    u8 pad000[0x030 - 0x000];
    /* +0x030 */ u_long *otSlot; /* the face's OT entry, TransformAndCullPoly's */
    u8 pad034[0x060 - 0x034];
    /* +0x060 */ DVECTOR sxy[4]; /* the face's screen XYs */
    /* +0x070 */ DVECTOR bboxMin;
    /* +0x074 */ DVECTOR bboxMax;
    /* +0x078 */ s32 divide; /* nonzero: submit through RCpoly* */
    u8 pad07C[0x088 - 0x07C];
    /* +0x088 */ RVECTOR *triVtx[3];  /* gDivPolygon3's r0..r2 */
    /* +0x094 */ RVECTOR *quadVtx[4]; /* gDivPolygon4's r0..r3 */
    /* +0x0A4 */ SVECTOR *srcVtx[4];  /* the face's model-space vertices */
} PolyDrawCtx;

/* Sony's prototypes, copied from libgte.h, where they sit commented out
 * because that header does not include libgpu.h's POLY_* types. */
extern u_long *RCpolyF3(POLY_F3 *s, DIVPOLYGON3 *divp);
extern u_long *RCpolyF4(POLY_F4 *s, DIVPOLYGON4 *divp);
extern u_long *RCpolyFT3(POLY_FT3 *s, DIVPOLYGON3 *divp);
extern u_long *RCpolyFT4(POLY_FT4 *s, DIVPOLYGON4 *divp);
extern u_long *RCpolyG3(POLY_G3 *s, DIVPOLYGON3 *divp);
extern u_long *RCpolyG4(POLY_G4 *s, DIVPOLYGON4 *divp);
extern u_long *RCpolyGT3(POLY_GT3 *s, DIVPOLYGON3 *divp);
extern u_long *RCpolyGT4(POLY_GT4 *s, DIVPOLYGON4 *divp);

void FillDivPolygonHeader(void *divp, PolyDrawCtx *ctx, CVECTOR *rgbc, s32 textured, u_short clut,
                          u_short tpage);
void FillRVectors3(RVECTOR **dst, SVECTOR **src, DVECTOR *sxy0, DVECTOR *sxy1, DVECTOR *sxy2);
void FillRVectors4(RVECTOR **dst, SVECTOR **src, DVECTOR *sxy0, DVECTOR *sxy1, DVECTOR *sxy2,
                   DVECTOR *sxy3);

/* ProjectQuadFace's storeSxy callback for a POLY_FT4: with storeFirst3, the
 * GTE's three screen XYs go to x0y0..x2y2; without it, the one just
 * transformed for the fourth vertex goes to x3y3. */
void StoreSxyPolyFT4(POLY_FT4 *prim, s32 storeFirst3) {
    if (storeFirst3) {
        gte_stsxy3_ft4(prim);
    } else {
        gte_stsxy2(&prim->x3);
    }
}

/* The same for a POLY_GT4. */
void StoreSxyPolyGT4(POLY_GT4 *prim, s32 storeFirst3) {
    if (storeFirst3) {
        gte_stsxy3_gt4(prim);
    } else {
        gte_stsxy2(&prim->x3);
    }
}

/*
 * Link `prim` into the OT, or subdivide it, and return the next free packet
 * address: prim + 1 after addPrim, or whatever RCpolyF3 returns.
 *
 * The eight wrappers differ only in which DIVPOLYGON they use and in how much
 * of the primitive goes into the RVECTORs besides model vertex and screen XY
 * (FillRVectors3/4): colour per vertex for Gouraud, UV per vertex and the
 * CLUT/TPAGE pair for textured.
 *
 * MATCHING: the divide arm must come first and return; with the addPrim arm
 * first the blocks swap.
 */
u_long *SubmitPolyF3(POLY_F3 *prim, PolyDrawCtx *ctx) {
    if (ctx->divide != 0) {
        FillDivPolygonHeader(gDivPolygon3, ctx, (CVECTOR *)&prim->r0, 0, 0, 0);
        FillRVectors3(ctx->triVtx, ctx->srcVtx, (DVECTOR *)&prim->x0, (DVECTOR *)&prim->x1,
                      (DVECTOR *)&prim->x2);
        return RCpolyF3(prim, (DIVPOLYGON3 *)gDivPolygon3);
    }
    addPrim(ctx->otSlot, prim);
    return (u_long *)(prim + 1);
}

u_long *SubmitPolyG3(POLY_G3 *prim, PolyDrawCtx *ctx) {
    if (ctx->divide != 0) {
        FillDivPolygonHeader(gDivPolygon3, ctx, (CVECTOR *)&prim->r0, 0, 0, 0);
        FillRVectors3(ctx->triVtx, ctx->srcVtx, (DVECTOR *)&prim->x0, (DVECTOR *)&prim->x1,
                      (DVECTOR *)&prim->x2);

        ctx->triVtx[0]->pad = prim->pad1;
        ctx->triVtx[1]->pad = prim->pad1;
        ctx->triVtx[2]->pad = prim->pad2;

        ctx->triVtx[0]->c = *(CVECTOR *)&prim->r0;
        ctx->triVtx[1]->c = *(CVECTOR *)&prim->r1;
        ctx->triVtx[2]->c = *(CVECTOR *)&prim->r2;

        return RCpolyG3(prim, (DIVPOLYGON3 *)gDivPolygon3);
    }
    addPrim(ctx->otSlot, prim);
    return (u_long *)(prim + 1);
}

u_long *SubmitPolyFT3(POLY_FT3 *prim, PolyDrawCtx *ctx) {
    if (ctx->divide != 0) {
        FillDivPolygonHeader(gDivPolygon3, ctx, (CVECTOR *)&prim->r0, 1, prim->clut, prim->tpage);
        FillRVectors3(ctx->triVtx, ctx->srcVtx, (DVECTOR *)&prim->x0, (DVECTOR *)&prim->x1,
                      (DVECTOR *)&prim->x2);

        ctx->triVtx[0]->pad = prim->pad1;
        ctx->triVtx[1]->pad = prim->pad1;
        ctx->triVtx[2]->pad = prim->pad1;
        *(u_short *)ctx->triVtx[0]->uv = *(u_short *)&prim->u0;
        *(u_short *)ctx->triVtx[1]->uv = *(u_short *)&prim->u1;
        *(u_short *)ctx->triVtx[2]->uv = *(u_short *)&prim->u2;

        return RCpolyFT3(prim, (DIVPOLYGON3 *)gDivPolygon3);
    }
    addPrim(ctx->otSlot, prim);
    return (u_long *)(prim + 1);
}

u_long *SubmitPolyF4(POLY_F4 *prim, PolyDrawCtx *ctx) {
    if (ctx->divide != 0) {
        FillDivPolygonHeader(gDivPolygon4, ctx, (CVECTOR *)&prim->r0, 0, 0, 0);
        FillRVectors4(ctx->quadVtx, ctx->srcVtx, (DVECTOR *)&prim->x0, (DVECTOR *)&prim->x1,
                      (DVECTOR *)&prim->x2, (DVECTOR *)&prim->x3);
        return RCpolyF4(prim, (DIVPOLYGON4 *)gDivPolygon4);
    }
    addPrim(ctx->otSlot, prim);
    return (u_long *)(prim + 1);
}

u_long *SubmitPolyG4(POLY_G4 *prim, PolyDrawCtx *ctx) {
    if (ctx->divide != 0) {
        FillDivPolygonHeader(gDivPolygon4, ctx, (CVECTOR *)&prim->r0, 0, 0, 0);
        FillRVectors4(ctx->quadVtx, ctx->srcVtx, (DVECTOR *)&prim->x0, (DVECTOR *)&prim->x1,
                      (DVECTOR *)&prim->x2, (DVECTOR *)&prim->x3);

        ctx->quadVtx[0]->pad = prim->pad1;
        ctx->quadVtx[1]->pad = prim->pad1;
        ctx->quadVtx[2]->pad = prim->pad2;
        ctx->quadVtx[3]->pad = prim->pad3;

        ctx->quadVtx[0]->c = *(CVECTOR *)&prim->r0;
        ctx->quadVtx[1]->c = *(CVECTOR *)&prim->r1;
        ctx->quadVtx[2]->c = *(CVECTOR *)&prim->r2;
        ctx->quadVtx[3]->c = *(CVECTOR *)&prim->r3;

        return RCpolyG4(prim, (DIVPOLYGON4 *)gDivPolygon4);
    }
    addPrim(ctx->otSlot, prim);
    return (u_long *)(prim + 1);
}

u_long *SubmitPolyFT4(POLY_FT4 *prim, PolyDrawCtx *ctx) {
    if (ctx->divide != 0) {
        FillDivPolygonHeader(gDivPolygon4, ctx, (CVECTOR *)&prim->r0, 1, prim->clut, prim->tpage);
        FillRVectors4(ctx->quadVtx, ctx->srcVtx, (DVECTOR *)&prim->x0, (DVECTOR *)&prim->x1,
                      (DVECTOR *)&prim->x2, (DVECTOR *)&prim->x3);

        ctx->quadVtx[0]->pad = prim->pad1;
        ctx->quadVtx[1]->pad = prim->pad1;
        ctx->quadVtx[2]->pad = prim->pad1;
        ctx->quadVtx[3]->pad = prim->pad1;
        *(u_short *)ctx->quadVtx[0]->uv = *(u_short *)&prim->u0;
        *(u_short *)ctx->quadVtx[1]->uv = *(u_short *)&prim->u1;
        *(u_short *)ctx->quadVtx[2]->uv = *(u_short *)&prim->u2;
        *(u_short *)ctx->quadVtx[3]->uv = *(u_short *)&prim->u3;

        return RCpolyFT4(prim, (DIVPOLYGON4 *)gDivPolygon4);
    }
    addPrim(ctx->otSlot, prim);
    return (u_long *)(prim + 1);
}

u_long *SubmitPolyGT3(POLY_GT3 *prim, PolyDrawCtx *ctx) {
    if (ctx->divide != 0) {
        FillDivPolygonHeader(gDivPolygon3, ctx, (CVECTOR *)&prim->r0, 1, prim->clut, prim->tpage);
        FillRVectors3(ctx->triVtx, ctx->srcVtx, (DVECTOR *)&prim->x0, (DVECTOR *)&prim->x1,
                      (DVECTOR *)&prim->x2);

        ctx->triVtx[0]->pad = prim->pad2;
        ctx->triVtx[1]->pad = prim->pad2;
        ctx->triVtx[2]->pad = prim->pad2;

        ctx->triVtx[0]->c = *(CVECTOR *)&prim->r0;
        ctx->triVtx[1]->c = *(CVECTOR *)&prim->r1;
        ctx->triVtx[2]->c = *(CVECTOR *)&prim->r2;

        *(u_short *)ctx->triVtx[0]->uv = *(u_short *)&prim->u0;
        *(u_short *)ctx->triVtx[1]->uv = *(u_short *)&prim->u1;
        *(u_short *)ctx->triVtx[2]->uv = *(u_short *)&prim->u2;

        return RCpolyGT3(prim, (DIVPOLYGON3 *)gDivPolygon3);
    }
    addPrim(ctx->otSlot, prim);
    return (u_long *)(prim + 1);
}

u_long *SubmitPolyGT4(POLY_GT4 *prim, PolyDrawCtx *ctx) {
    if (ctx->divide != 0) {
        FillDivPolygonHeader(gDivPolygon4, ctx, (CVECTOR *)&prim->r0, 1, prim->clut, prim->tpage);
        FillRVectors4(ctx->quadVtx, ctx->srcVtx, (DVECTOR *)&prim->x0, (DVECTOR *)&prim->x1,
                      (DVECTOR *)&prim->x2, (DVECTOR *)&prim->x3);

        ctx->quadVtx[0]->pad = prim->pad2;
        ctx->quadVtx[1]->pad = prim->pad2;
        ctx->quadVtx[2]->pad = prim->pad3;
        ctx->quadVtx[3]->pad = prim->pad3;

        ctx->quadVtx[0]->c = *(CVECTOR *)&prim->r0;
        ctx->quadVtx[1]->c = *(CVECTOR *)&prim->r1;
        ctx->quadVtx[2]->c = *(CVECTOR *)&prim->r2;
        ctx->quadVtx[3]->c = *(CVECTOR *)&prim->r3;

        *(u_short *)ctx->quadVtx[0]->uv = *(u_short *)&prim->u0;
        *(u_short *)ctx->quadVtx[1]->uv = *(u_short *)&prim->u1;
        *(u_short *)ctx->quadVtx[2]->uv = *(u_short *)&prim->u2;
        *(u_short *)ctx->quadVtx[3]->uv = *(u_short *)&prim->u3;

        return RCpolyGT4(prim, (DIVPOLYGON4 *)gDivPolygon4);
    }
    addPrim(ctx->otSlot, prim);
    return (u_long *)(prim + 1);
}

/*
 * Point a draw context's RVECTOR list (`vtxPtrs`, its triVtx or quadVtx)
 * and the DIVPOLYGON's own first recursion level (cr[0].r0...) at that
 * DIVPOLYGON's vertex records r0, r1, ... `nverts` is 3 for a DIVPOLYGON3,
 * 4 for a DIVPOLYGON4; code_8220_b calls it for both, once per object.
 */
void InitDivPolygonPtrs(RVECTOR **vtxPtrs, void *divp, s32 nverts) {
    RVECTOR *rv = &((DIVPOLYGON3 *)divp)->r0;
    RVECTOR **ctxPtr = vtxPtrs;
    RVECTOR **crPtr =
        (nverts == 4) ? &((DIVPOLYGON4 *)divp)->cr[0].r0 : &((DIVPOLYGON3 *)divp)->cr[0].r0;

    while (nverts-- > 0) {
        *crPtr = rv;
        *ctxPtr = rv;
        rv++;
        crPtr++;
        ctxPtr++;
    }
}

/*
 * Take the screen bounding box of the face's cached XYs into bboxMin/bboxMax
 * and set `divide` when it is wider or taller than MAX_UNDIVIDED_SPAN.
 * `count` is the vertex count, 3 or 4. The loop stops one vertex short:
 * the last vertex never enters the box (retail's behaviour).
 */
void FlagLargePolyForDivide(void *ctxIn, s32 count) {
    PolyDrawCtx *ctx = ctxIn;
    short *xp, *yp, *end;

    ctx->bboxMax = ctx->sxy[0];
    ctx->bboxMin = ctx->bboxMax;

    xp = &ctx->sxy[1].vx;
    /* MATCHING: this is &ctx->sxy[count - 1].vx, but that spelling folds the
     * 0x5C into the index before adding ctx, and retail adds ctx first. */
    end = (short *)((u8 *)ctx + count * sizeof(DVECTOR) + 0x5C);

    /* MATCHING: a guarded do/while with yp set inside the guard. Setting yp
     * before the test moves the frame setup out of the branch delay slot. */
    if (xp < end) {
        yp = &ctx->sxy[1].vy;
        do {
            if (*xp < ctx->bboxMin.vx) {
                ctx->bboxMin.vx = *xp;
            }
            if (*yp < ctx->bboxMin.vy) {
                ctx->bboxMin.vy = *yp;
            }
            if (ctx->bboxMax.vx < *xp) {
                ctx->bboxMax.vx = *xp;
            }
            if (ctx->bboxMax.vy < *yp) {
                ctx->bboxMax.vy = *yp;
            }
            xp += 2;
            yp += 2;
        } while (xp < end);
    }

    if (ctx->bboxMax.vx - ctx->bboxMin.vx > MAX_UNDIVIDED_SPAN) {
        ctx->divide = 1;
    }
    if (ctx->bboxMax.vy - ctx->bboxMin.vy > MAX_UNDIVIDED_SPAN) {
        ctx->divide = 1;
    }
}

/* The clip area every DIVPOLYGON gets: 320 x 240, the screen. */
extern s32 sDivClipWidth;
extern s32 sDivClipHeight;

/* The object's default ndiv: code_8220_b's func_80018464 sets it from bits
 * 9-11 of the drawn object's flags. It keeps its address-style name because
 * config/psyq-objects.ld's `dc_cb` pin covers the address; see the report. */
extern s32 D_80090C18;

/* SetNdivOverride's: when set, sNdivOverride replaces D_80090C18. */
extern s32 sNdivOverrideSet;
extern s32 sNdivOverride;

/*
 * Fill the header of a DIVPOLYGON3 or DIVPOLYGON4 (the two share it):
 * ndiv, the clip area, the primitive's colour word and the face's OT entry,
 * and for a textured primitive its CLUT and TPAGE.
 */
void FillDivPolygonHeader(void *divpIn, PolyDrawCtx *ctx, CVECTOR *rgbc, s32 textured, u_short clut,
                          u_short tpage) {
    DIVPOLYGON3 *divp = divpIn;
    s32 ndiv;
    s32 pih;
    s32 piv;

    /* MATCHING: one store of a temp after the if/else, and the clip area read
     * after it; a ternary or a store per arm costs a word. */
    if (sNdivOverrideSet) {
        ndiv = sNdivOverride;
    } else {
        ndiv = D_80090C18;
    }
    pih = sDivClipWidth;
    piv = sDivClipHeight;

    divp->ndiv = ndiv;
    divp->pih = pih;
    divp->piv = piv;

    if (textured != 0) {
        divp->clut = clut;
        divp->tpage = tpage;
    }

    divp->rgbc = *rgbc;
    divp->ot = ctx->otSlot;
}

/*
 * Give three RVECTORs their model-space vertex and screen XY.
 *
 * MATCHING: whole-struct assignments. SVECTOR and DVECTOR have only short
 * members, so their alignment is 2 and GCC copies them with lwl/lwr,
 * swl/swr, as retail does.
 */
void FillRVectors3(RVECTOR **dst, SVECTOR **src, DVECTOR *sxy0, DVECTOR *sxy1, DVECTOR *sxy2) {
    dst[0]->v = *src[0];
    dst[1]->v = *src[1];
    dst[2]->v = *src[2];
    dst[0]->sxy = *sxy0;
    dst[1]->sxy = *sxy1;
    dst[2]->sxy = *sxy2;
}

/* The same for four. */
void FillRVectors4(RVECTOR **dst, SVECTOR **src, DVECTOR *sxy0, DVECTOR *sxy1, DVECTOR *sxy2,
                   DVECTOR *sxy3) {
    FillRVectors3(dst, src, sxy0, sxy1, sxy2);
    dst[3]->v = *src[3];
    dst[3]->sxy = *sxy3;
}

/* Make FillDivPolygonHeader use `ndiv` instead of each object's default,
 * or (enable == 0) go back to the default. Nothing in the image calls it. */
void SetNdivOverride(s32 enable, s32 ndiv) {
    sNdivOverrideSet = enable;
    if (enable) {
        sNdivOverride = ndiv;
    }
}
