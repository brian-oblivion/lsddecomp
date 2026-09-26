/*
 * code_8220_c -- third slice of the code_8220 block (carved round 13): the
 * eight "submit wrappers" code_8220_b's own comment names them (each a
 * tail call to one of Sony's RCpoly* polygon-subdivision packers, or a
 * direct OT splice), the two flag-gated GTE screen-XY store leaves that
 * complete the six-member StoreSxyPoly** family split across this unit and
 * code_8220_b, and the small helpers those wrappers share.
 */

#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include "code_8220.h"
#include "gte.h"

typedef struct PolyDrawCtx {
    u8 pad000[0x030 - 0x000];
    /* +0x030 */ u_long *otSlot;
    u8 pad034[0x060 - 0x034];
    /* +0x060 */ DVECTOR sxy[4];
    /* +0x070 */ DVECTOR bboxMin;
    /* +0x074 */ DVECTOR bboxMax;
    /* +0x078 */ s32 divide;
    u8 pad07C[0x088 - 0x07C];
    /* +0x088 */ RVECTOR *triVtx[3];
    /* +0x094 */ RVECTOR *quadVtx[4];
    /* +0x0A4 */ SVECTOR *srcVtx[4];
} PolyDrawCtx;

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

void StoreSxyPolyFT4(POLY_FT4 *prim, s32 storeFirst3) {
    if (storeFirst3) {
        gte_stsxy3_ft4(prim);
    } else {
        short *p = &prim->x3;

        gte_stsxy2(p);
    }
}

void StoreSxyPolyGT4(POLY_GT4 *prim, s32 storeFirst3) {
    if (storeFirst3) {
        gte_stsxy3_gt4(prim);
    } else {
        short *p = &prim->x3;

        gte_stsxy2(p);
    }
}

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

void InitDivPolygonPtrs(RVECTOR **dst, void *divp, s32 kind) {
    RVECTOR *src = &((DIVPOLYGON3 *)divp)->r0;
    RVECTOR **dst0 = dst;
    RVECTOR **dst1 = (kind == 4) ? &((DIVPOLYGON4 *)divp)->cr[0].r0 : &((DIVPOLYGON3 *)divp)->cr[0].r0;

    while (kind-- > 0) {
        *dst1 = src;
        *dst0 = src;
        src++;
        dst1++;
        dst0++;
    }
}

void FlagLargePolyForDivide(void *ctxIn, s32 count) {
    PolyDrawCtx *ctx = ctxIn;
    short *xp, *yp, *end;

    ctx->bboxMax = ctx->sxy[0];
    ctx->bboxMin = ctx->bboxMax;

    xp = &ctx->sxy[1].vx;
    /* MATCHING: this is &ctx->sxy[count - 1].vx, but that spelling folds the
     * 0x5C into the index before adding ctx, and retail adds ctx first. */
    end = (short *)((u8 *)ctx + count * sizeof(DVECTOR) + 0x5C);

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

    if (ctx->bboxMax.vx - ctx->bboxMin.vx >= 0x101) {
        ctx->divide = 1;
    }
    if (ctx->bboxMax.vy - ctx->bboxMin.vy >= 0x101) {
        ctx->divide = 1;
    }
}

extern s32 sDivClipWidth;
extern s32 sDivClipHeight;

extern s32 D_80090C18;

extern s32 sNdivOverrideSet;
extern s32 sNdivOverride;

void FillDivPolygonHeader(void *divp, PolyDrawCtx *ctx, CVECTOR *rgbc, s32 textured, u_short clut,
                          u_short tpage) {
    DIVPOLYGON3 *dst = divp;
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

    dst->ndiv = val;
    dst->pih = code;
    dst->piv = code2;

    if (textured != 0) {
        dst->clut = clut;
        dst->tpage = tpage;
    }

    dst->rgbc = *rgbc;
    dst->ot = ctx->otSlot;
}

void FillRVectors3(RVECTOR **dst, SVECTOR **src, DVECTOR *sxy0, DVECTOR *sxy1, DVECTOR *sxy2) {
    dst[0]->v = *src[0];
    dst[1]->v = *src[1];
    dst[2]->v = *src[2];
    dst[0]->sxy = *sxy0;
    dst[1]->sxy = *sxy1;
    dst[2]->sxy = *sxy2;
}

void FillRVectors4(RVECTOR **dst, SVECTOR **src, DVECTOR *sxy0, DVECTOR *sxy1, DVECTOR *sxy2,
                   DVECTOR *sxy3) {
    FillRVectors3(dst, src, sxy0, sxy1, sxy2);
    dst[3]->v = *src[3];
    dst[3]->sxy = *sxy3;
}

void SetNdivOverride(s32 enable, s32 code) {
    sNdivOverrideSet = enable;
    if (enable) {
        sNdivOverride = code;
    }
}
