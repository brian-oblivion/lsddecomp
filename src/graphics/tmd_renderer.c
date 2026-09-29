/*
 * tmd_renderer.c -- the game's TMD renderer.
 *
 * SortTmdObject, the game's replacement for Sony's
 * GsSortObject4, turns a GsDOBJ2's TMD object into GPU primitives. Per face,
 * SetupPrimCode finishes the primitive's command byte, ProjectTriFace or
 * ProjectQuadFace transforms and culls it through TransformAndCullPoly,
 * writes its screen coordinates with one of the StoreSxyPoly* leaves and
 * takes its screen box (FlagLargePolyForDivide), and a SubmitPoly* wrapper
 * puts the finished primitive into the ordering table: straight into its OT
 * slot with addPrim when the box fits, otherwise (too large on screen, or a
 * saturated screen Z) copied into one of the two DIVPOLYGON work buffers
 * and handed to Sony's RCpoly* packer, which subdivides it. The file ends
 * with the helpers that fill a DIVPOLYGON's header and its RVECTOR vertex
 * records, and the ndiv override setter. All GTE work goes through
 * include/gte.h's gte_* macros (Sony's names; never <inline.h>).
 */

#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "draw_system.h"
#include "scene_node.h"
#include "gte.h"
#include "tmd_renderer.h"

/* The drawn object's attribute bits, as SortTmdObject publishes them for
 * SetupPrimCode and the submit wrappers. Sony's GsSortObject4 keeps the same
 * four fields in GsNDIV, GsLIOFF, GsLIGNR and GsLMODE. sSortLightOff is bit 6
 * of the object's attribute word, which SetupPrimCode ORs into the GPU
 * command byte's shade-texture bit 0x1 (Psy-Q's SetShadeTex). sSortNdiv is
 * the object's subdivision level, bits 9-11, the ndiv every DIVPOLYGON gets
 * unless SetNdivOverride has set one. */
extern s32 sSortLightOff;           /* GsLOFF */
extern s32 sSortNdiv;               /* GsDIV1..5: subdivision level */
extern s32 sSortUseGlobalLightMode; /* GsLLMOD */
extern s32 sSortLightMode;          /* GsFOG | GsMATE */

/**
 * @brief The per-object draw context SortTmdObject builds in the PS1
 * scratchpad (its caller passes 0x1F800000) and hands to every function
 * below. Only the fields this file touches are named.
 */
typedef struct PolyDrawCtx {
    /* +0x000 */ GsOT_TAG *otBase;  /**< the GsOT's org */
    /* +0x004 */ s32 otShift;       /**< otz >> otShift indexes otBase */
    /* +0x008 */ s32 unk8;          /**< set to 10 per object; nothing reads it */
    /* +0x00C */ SVECTOR *vertices; /**< the TMD object's vertex array */
    /* +0x010 */ SVECTOR *normals;  /**< the TMD object's normal array */
    /* +0x014 */ u8 primLen;        /**< SetupPrimCode's cached P_TAG length byte */
    /* +0x015 */ u8 primCode;       /**< ... and finished GPU command byte */
    u8 pad016[0x018 - 0x016];
    /* +0x018 */ u32 packetType;     /**< the current group's TMD mode/flag word */
    /* +0x01C */ s32 semiTrans;      /**< the group's ABE bit */
    /* +0x020 */ s32 otz;            /**< the face's average screen depth */
    /* +0x024 */ s32 dp;             /**< the face's depth-cue factor; ONE or more culls it */
    /* +0x028 */ s32 opz;            /**< the face's winding; 0 or less culls it as back-facing */
    /* +0x02C */ s32 dpShift;        /**< dp >> dpShift is the CLUT row offset */
    /* +0x030 */ GsOT_TAG *otSlot;   /**< &otBase[otz >> otShift] */
    /* +0x034 */ ColorRgb faceColor; /**< sTexturedFaceColor's copy */
    u8 pad037[0x038 - 0x037];
    /* +0x038 */ MATRIX savedRotMatrix; /**< the rotation matrix, saved while a child's is concatenated with its parent's */
    u8 pad058[0x05C - 0x058];
    /* +0x05C */ s32 flag; /**< the projection's error flags: SZ3/OTZ saturation alone forces subdivision, anything else culls */
    /* +0x060 */ DVECTOR sxy[4];  /**< the face's screen XYs; [3] only for a quad */
    /* +0x070 */ DVECTOR bboxMin; /**< FlagLargePolyForDivide's screen box, top left */
    /* +0x074 */ DVECTOR bboxMax; /**< ... and bottom right */
    /* +0x078 */ s32 divide;      /**< set when the face must go through RCpoly* subdivision */
    u8 pad07C[0x088 - 0x07C];
    /* +0x088 */ RVECTOR *divVtx3[3]; /**< sDivPolygon3's r0..r2 */
    /* +0x094 */ RVECTOR *divVtx4[4]; /**< sDivPolygon4's r0..r3 */
    /* +0x0A4 */ SVECTOR *faceVtx[4]; /**< the current face's vertices */
} PolyDrawCtx;

/** @brief The first word of a group's first TMD packet. Sony's TMD_P_*
 * structs spell it as four bytes (out, in, dummy, cd: olen, ilen, flag,
 * mode); this renderer reads the first two as the group's packet count and
 * the last two as one flag | mode << 8 word. */
typedef struct TmdGroupHeader {
    u16 count; /**< how many packets the group holds */
    u16 type;  /**< the group's packets' flag | mode << 8, as TMD_TYPE builds it */
} TmdGroupHeader;

/* A TMD packet's type as SortTmdObject switches on it: the mode byte
 * (Sony's GPU_COM_* codes, <libgs.h>) above the flag byte. */
#define TMD_TYPE(mode, flag) ((mode) << 8 | (flag))
#define TMD_FLAG_LGT 0x01    /* flag: light source calculation off */
#define TMD_TYPE_MASK 0xFD07 /* mode less its ABE bit 0x02; flag's LGT, FCE, GRD */

/* The mode byte's ABE (semi-transparency) bit, read in the packet's first
 * word: mode is its top byte, ABE that byte's bit 1. */
#define TMD_WORD_ABE_SHIFT 25

/* A textured face's CLUT is moved down dp >> shift palette rows (dp, the
 * GTE depth-cue factor, is below ONE, 4096): 9 gives 0..7 rows of
 * depth-cued palettes for an object whose light mode is not plain (GsFOG,
 * GsMATE, or GsLIGHT_MODE through GsLLMOD), 16 gives none. */
#define DP_CLUT_SHIFT_CUED 9
#define DP_CLUT_SHIFT_NONE 16

/* GTE FLAG bit 18: SZ3 or OTZ saturated. The one FLAG state
 * TransformAndCullPoly keeps a polygon for, routed to subdivision. */
#define GTE_FLAG_SZ3_OTZ_SAT 0x40000

/* GsDOBJ2 keeps its TMD object as a u_long *. */
#define OBJ_TMD(obj) ((struct TMD_STRUCT *)(obj)->tmd)

/* Move a textured primitive's CLUT `rows` palette rows down (one row is
 * 1 << 6 in libgpu's getClut encoding). */
/* MATCHING: the do/while (0); without it all six uses compile differently */
/* clang-format off */
#define ADD_CLUT_ROWS(p, rows) \
    do { \
        ((POLY_FT3 *)(p))->clut += (rows) << 6; \
    } while (0)
/* clang-format on */
extern ColorRgb sTexturedFaceColor;

/* The two subdivision work buffers the SubmitPoly* wrappers hand Sony's
 * RCpoly* packers, a DIVPOLYGON3 and a DIVPOLYGON4 back to back (0x218
 * bytes apart, sizeof(DIVPOLYGON3)). Declared as bytes: InitDivPolygonPtrs
 * takes their addresses and the wrappers cast. */
extern u8 sDivPolygon3[];
extern u8 sDivPolygon4[];

/* Defined at the bottom of this file, after SortTmdObject and
 * ProjectQuadFace, which call them. The submit wrappers link the finished
 * primitive into the OT (directly, or through its RCpoly* subdivider) and
 * return the packet cursor past what they wrote. SortTmdObject's cursor is
 * a u8 *, so its calls cast it to each wrapper's POLY_* type. */
void InitDivPolygonPtrs(RVECTOR **vtxPtrs, void *divp, s32 nverts);
void StoreSxyPolyFT4(POLY_FT4 *prim, s32 storeFirst3);
void StoreSxyPolyGT4(POLY_GT4 *prim, s32 storeFirst3);
void *SubmitPolyF3(POLY_F3 *prim, PolyDrawCtx *ctx);
void *SubmitPolyG3(POLY_G3 *prim, PolyDrawCtx *ctx);
void *SubmitPolyFT3(POLY_FT3 *prim, PolyDrawCtx *ctx);
void *SubmitPolyF4(POLY_F4 *prim, PolyDrawCtx *ctx);
void *SubmitPolyG4(POLY_G4 *prim, PolyDrawCtx *ctx);
void *SubmitPolyFT4(POLY_FT4 *prim, PolyDrawCtx *ctx);
void *SubmitPolyGT3(POLY_GT3 *prim, PolyDrawCtx *ctx);
void *SubmitPolyGT4(POLY_GT4 *prim, PolyDrawCtx *ctx);

/* Defined below SortTmdObject, which calls them (and ProjectTriFace and
 * ProjectQuadFace, which call the last two). */
void SetupPrimCode(void *prim, PolyDrawCtx *ctx);
s32 ProjectTriFace(void *prim, PolyDrawCtx *ctx, u16 idx0, u16 idx1, u16 idx2, void (*storeSxy)(void *));
s32 ProjectQuadFace(void *prim, PolyDrawCtx *ctx, u16 idx0, u16 idx1, u16 idx2, u16 idx3,
                    void (*storeSxy)(void *, s32));
void StoreSxyPolyF3(void *dst);
void StoreSxyPolyG3(void *dst);
void StoreSxyPolyFT3(void *dst);
void StoreSxyPolyGT3(void *dst);
void StoreSxyPolyF4(void *dst, s32 storeFirst3);
void StoreSxyPolyG4(void *dst, s32 storeFirst3);
s32 TransformAndCullPoly(void *prim, void *ctx);
void FlagLargePolyForDivide(void *ctx, s32 count);

/*
 * The game's own GsSortObject4 (same arguments): emit every surviving face of
 * `obj`'s TMD object into `ot` as a GPU primitive in the GsOUT_PACKET_P
 * buffer. `otShift` turns a face's Z into its OT slot; `scratch` is where the
 * PolyDrawCtx goes.
 *
 * First the object's attribute bits go to the globals above, and when its
 * GsCOORDINATE2 has a parent, the object's workm is rotated by the parent's
 * workm column by column, the GTE rotation matrix saved around it. The TMD's
 * primitives come in runs of one packet type, the first packet's header
 * holding the run's length; each type has one case, which sets the
 * primitive's length and code once, then per packet projects the face and,
 * if it survives, lights it from its normal (ncds) or depth-cues its own
 * colours (dpcs, dpct), copies the UVs and hands the primitive to its submit
 * wrapper. The packet cursor is reloaded from GsOUT_PACKET_P per run and
 * written back after it; an unknown type ends the object.
 *
 * `packet` is the current TMD packet and `elem` walks beside it, parked on
 * one member; PKT names the packet as seen from `elem`, POLY the primitive.
 */
/* MATCHING: retail's shape needs the two packet cursors, a goto loop per case,
 * ctx set only after the GsDOFF test, and the colour stores through &POLY->r0 */
void SortTmdObject(GsDOBJ2 *obj, GsOT *ot, s32 otShift, void *scratch) {
    PolyDrawCtx *ctx;
    u8 *prim;
    u8 *packet;
    s32 packetsLeft;
    s32 dpShift;

    if (obj->attribute & GsDOFF) {
        return;
    }

    ctx = scratch;

    ctx->otBase = ot->org;
    ctx->otShift = otShift;
    InitDivPolygonPtrs(ctx->divVtx3, sDivPolygon3, ARRAY_COUNT(ctx->divVtx3));
    InitDivPolygonPtrs(ctx->divVtx4, sDivPolygon4, ARRAY_COUNT(ctx->divVtx4));

    packetsLeft = OBJ_TMD(obj)->primn;
    packet = (u8 *)OBJ_TMD(obj)->primtop;
    ctx->vertices = (SVECTOR *)OBJ_TMD(obj)->vertop;
    ctx->normals = (SVECTOR *)OBJ_TMD(obj)->nortop;

    if (obj->coord2->super != NULL) {
        gte_ReadRotMatrix(&ctx->savedRotMatrix);
        gte_SetRotMatrix(&obj->coord2->super->workm);

        gte_ldclmv(&obj->coord2->workm.m[0][0]);
        gte_llir();
        gte_stclmv(&obj->coord2->workm.m[0][0]);

        gte_ldclmv(&obj->coord2->workm.m[0][1]);
        gte_llir();
        gte_stclmv(&obj->coord2->workm.m[0][1]);

        gte_ldclmv(&obj->coord2->workm.m[0][2]);
        gte_llir();
        gte_stclmv(&obj->coord2->workm.m[0][2]);

        gte_SetRotMatrix(&ctx->savedRotMatrix);
    }

    /* MATCHING: obj->attribute read afresh for each global, not once into a local;
     * all four reads come before the first store, and unk8 is stored after them. */
    sSortNdiv = (obj->attribute >> ATTR_DIV_SHIFT) & 0x7;
    sSortLightOff = (obj->attribute >> ATTR_LOFF_SHIFT) & 0x1;
    sSortUseGlobalLightMode = (obj->attribute >> ATTR_LLMOD_SHIFT) & 0x1;
    sSortLightMode = (obj->attribute >> ATTR_LIGHTMODE_SHIFT) & 0x3;
    ctx->unk8 = 10;

    ctx->faceColor = sTexturedFaceColor;

    if ((sSortUseGlobalLightMode != 0 && GsLIGHT_MODE != 0) || sSortLightMode != 0) {
        dpShift = DP_CLUT_SHIFT_CUED;
    } else {
        dpShift = DP_CLUT_SHIFT_NONE;
    }
    ctx->dpShift = dpShift;

    if (packetsLeft == 0) {
        return;
    }

    {
        void (*storeSxyG3)(void *) = StoreSxyPolyG3;

        do {
            s32 count;

            prim = GsOUT_PACKET_P;
            ctx->packetType = ((TmdGroupHeader *)packet)->type & TMD_TYPE_MASK;
            count = ((TmdGroupHeader *)packet)->count;
            ctx->semiTrans = (*(u32 *)packet >> TMD_WORD_ABE_SHIFT) & 0x1;
            packetsLeft -= count;

            /* MATCHING: cases stay in ascending type order (the bodies are laid out in source order). */
            switch (ctx->packetType) {
                case TMD_TYPE(GPU_COM_F3, 0): {
                    u8 *elem;
#define PKT ((TMD_P_F3 *)(elem - offsetof(TMD_P_F3, r0)))
#define POLY ((POLY_F3 *)prim)

                    /* TMD_P_F3 -> POLY_F3: one colour, lit from the face normal. */
                    setPolyF3(prim);
                    SetupPrimCode(prim, ctx);
                    elem = packet + offsetof(TMD_P_F3, r0);
                loopA:
                    if (ProjectTriFace(prim, ctx, PKT->v0, PKT->v1, PKT->v2, StoreSxyPolyF3) == 0) {
                        gte_ldv0(&ctx->normals[PKT->n0]);
                        gte_ldrgb(&PKT->r0);
                        gte_ncds();
                        gte_strgb(&POLY->r0);
                        setcode(prim, ctx->primCode);
                        prim = SubmitPolyF3((POLY_F3 *)prim, ctx);
                    }
                    elem += sizeof(TMD_P_F3);
                    packet += sizeof(TMD_P_F3);
                    if (--count != 0)
                        goto loopA;
                    break;
#undef PKT
#undef POLY
                }

                case TMD_TYPE(GPU_COM_F3, GsTMDFlagGRD): {
                    u8 *elem;
#define PKT ((TMD_P_F3G *)(elem - offsetof(TMD_P_F3G, r2)))
#define POLY ((POLY_G3 *)prim)

                    /* TMD_P_F3G -> POLY_G3: flat-shaded with a colour per vertex (GRD),
                     * each lit from the one face normal. */
                    /* MATCHING: r0 and r1 are read through `packet`, r2 through `elem` */
                    setPolyG3(prim);
                    SetupPrimCode(prim, ctx);
                    elem = packet + offsetof(TMD_P_F3G, r2);
                loopB:
                    if (ProjectTriFace(prim, ctx, PKT->v0, PKT->v1, PKT->v2, storeSxyG3) == 0) {
                        gte_ldv0(&ctx->normals[PKT->n0]);
                        gte_ldrgb(&((TMD_P_F3G *)packet)->r0);
                        gte_ncds();
                        gte_strgb(&POLY->r0);
                        setcode(prim, ctx->primCode);
                        gte_ldrgb(&((TMD_P_F3G *)packet)->r1);
                        gte_ncds();
                        gte_strgb(&POLY->r1);
                        gte_ldrgb(&PKT->r2);
                        gte_ncds();
                        gte_strgb(&POLY->r2);
                        prim = SubmitPolyG3((POLY_G3 *)prim, ctx);
                    }
                    elem += sizeof(TMD_P_F3G);
                    packet += sizeof(TMD_P_F3G);
                    if (--count != 0)
                        goto loopB;
                    break;
#undef PKT
#undef POLY
                }

                case TMD_TYPE(GPU_COM_NF3, TMD_FLAG_LGT): {
                    u8 *elem;
#define PKT ((TMD_P_NF3 *)(elem - offsetof(TMD_P_NF3, r0)))
#define POLY ((POLY_F3 *)prim)

                    /* TMD_P_NF3 -> POLY_F3: unlit, its colour depth-cued. */
                    setPolyF3(prim);
                    SetupPrimCode(prim, ctx);
                    elem = packet + offsetof(TMD_P_NF3, r0);
                loopC:
                    if (ProjectTriFace(prim, ctx, PKT->v0, PKT->v1, PKT->v2, StoreSxyPolyF3) == 0) {
                        gte_ldrgb(&PKT->r0);
                        gte_dpcs();
                        gte_strgb(&POLY->r0);
                        setcode(prim, ctx->primCode);
                        prim = SubmitPolyF3((POLY_F3 *)prim, ctx);
                    }
                    elem += sizeof(TMD_P_NF3);
                    packet += sizeof(TMD_P_NF3);
                    if (--count != 0)
                        goto loopC;
                    break;
#undef PKT
#undef POLY
                }

                case TMD_TYPE(GPU_COM_TF3, 0): {
                    u8 *elem;
#define PKT ((TMD_P_TF3 *)(elem - offsetof(TMD_P_TF3, n0)))
#define POLY ((POLY_FT3 *)prim)

                    /* TMD_P_TF3 -> POLY_FT3: lit, from sTexturedFaceColor, which is
                     * loaded into the GTE once per run. */
                    setPolyFT3(prim);
                    SetupPrimCode(prim, ctx);
                    gte_ldrgb(&ctx->faceColor);
                    elem = packet + offsetof(TMD_P_TF3, n0);
                loopD:
                    if (ProjectTriFace(prim, ctx, PKT->v0, PKT->v1, PKT->v2, StoreSxyPolyFT3) == 0) {
                        *(u32 *)&POLY->u0 = *(u32 *)&PKT->tu0;
                        *(u32 *)&POLY->u1 = *(u32 *)&PKT->tu1;
                        *(u32 *)&POLY->u2 = *(u32 *)&PKT->tu2;
                        ADD_CLUT_ROWS(prim, ctx->dp >> ctx->dpShift);
                        gte_ldv0(&ctx->normals[PKT->n0]);
                        gte_ncds();
                        gte_strgb(&POLY->r0);
                        setcode(prim, ctx->primCode);
                        prim = SubmitPolyFT3((POLY_FT3 *)prim, ctx);
                    }
                    elem += sizeof(TMD_P_TF3);
                    packet += sizeof(TMD_P_TF3);
                    if (--count != 0)
                        goto loopD;
                    break;
#undef PKT
#undef POLY
                }

                case TMD_TYPE(GPU_COM_NTF3, TMD_FLAG_LGT): {
                    u8 *elem;
#define PKT ((TMD_P_TNF3 *)(elem - offsetof(TMD_P_TNF3, r0)))
#define POLY ((POLY_FT3 *)prim)

                    /* TMD_P_TNF3 -> POLY_FT3: unlit, its colour depth-cued. */
                    setPolyFT3(prim);
                    SetupPrimCode(prim, ctx);
                    elem = packet + offsetof(TMD_P_TNF3, r0);
                loopE:
                    if (ProjectTriFace(prim, ctx, PKT->v0, PKT->v1, PKT->v2, StoreSxyPolyFT3) == 0) {
                        *(u32 *)&POLY->u0 = *(u32 *)&PKT->tu0;
                        *(u32 *)&POLY->u1 = *(u32 *)&PKT->tu1;
                        *(u32 *)&POLY->u2 = *(u32 *)&PKT->tu2;
                        ADD_CLUT_ROWS(prim, ctx->dp >> ctx->dpShift);
                        gte_ldrgb(&PKT->r0);
                        gte_dpcs();
                        gte_strgb(&POLY->r0);
                        setcode(prim, ctx->primCode);
                        prim = SubmitPolyFT3((POLY_FT3 *)prim, ctx);
                    }
                    elem += sizeof(TMD_P_TNF3);
                    packet += sizeof(TMD_P_TNF3);
                    if (--count != 0)
                        goto loopE;
                    break;
#undef PKT
#undef POLY
                }

                case TMD_TYPE(GPU_COM_F4, 0): {
                    u8 *elem;
#define PKT ((TMD_P_F4 *)(elem - offsetof(TMD_P_F4, r0)))
#define POLY ((POLY_F4 *)prim)

                    /* TMD_P_F4 -> POLY_F4: one colour, lit from the face normal. */
                    setPolyF4(prim);
                    SetupPrimCode(prim, ctx);
                    elem = packet + offsetof(TMD_P_F4, r0);
                loopF:
                    if (ProjectQuadFace(prim, ctx, PKT->v0, PKT->v1, PKT->v2, PKT->v3,
                                        StoreSxyPolyF4) == 0) {
                        gte_ldv0(&ctx->normals[PKT->n0]);
                        gte_ldrgb(&PKT->r0);
                        gte_ncds();
                        gte_strgb(&POLY->r0);
                        setcode(prim, ctx->primCode);
                        prim = SubmitPolyF4((POLY_F4 *)prim, ctx);
                    }
                    elem += sizeof(TMD_P_F4);
                    packet += sizeof(TMD_P_F4);
                    if (--count != 0)
                        goto loopF;
                    break;
#undef PKT
#undef POLY
                }

                case TMD_TYPE(GPU_COM_NF4, TMD_FLAG_LGT): {
                    u8 *elem;
#define PKT ((TMD_P_NF4 *)(elem - offsetof(TMD_P_NF4, r0)))
#define POLY ((POLY_F4 *)prim)

                    /* TMD_P_NF4 -> POLY_F4: unlit, its colour depth-cued. */
                    setPolyF4(prim);
                    SetupPrimCode(prim, ctx);
                    elem = packet + offsetof(TMD_P_NF4, r0);
                loopG:
                    if (ProjectQuadFace(prim, ctx, PKT->v0, PKT->v1, PKT->v2, PKT->v3,
                                        StoreSxyPolyF4) == 0) {
                        gte_ldrgb(&PKT->r0);
                        gte_dpcs();
                        gte_strgb(&POLY->r0);
                        setcode(prim, ctx->primCode);
                        prim = SubmitPolyF4((POLY_F4 *)prim, ctx);
                    }
                    elem += sizeof(TMD_P_NF4);
                    packet += sizeof(TMD_P_NF4);
                    if (--count != 0)
                        goto loopG;
                    break;
#undef PKT
#undef POLY
                }

                case TMD_TYPE(GPU_COM_TF4, 0): {
                    u8 *elem;
#define PKT ((TMD_P_TF4 *)(elem - offsetof(TMD_P_TF4, n0)))
#define POLY ((POLY_FT4 *)prim)

                    /* TMD_P_TF4 -> POLY_FT4: lit, from sTexturedFaceColor. */
                    setPolyFT4(prim);
                    SetupPrimCode(prim, ctx);
                    gte_ldrgb(&ctx->faceColor);
                    elem = packet + offsetof(TMD_P_TF4, n0);
                loopH:
                    if (ProjectQuadFace(prim, ctx, PKT->v0, PKT->v1, PKT->v2, PKT->v3,
                                        (void (*)(void *, s32))StoreSxyPolyFT4) == 0) {
                        *(u32 *)&POLY->u0 = *(u32 *)&PKT->tu0;
                        *(u32 *)&POLY->u1 = *(u32 *)&PKT->tu1;
                        *(u32 *)&POLY->u2 = *(u32 *)&PKT->tu2;
                        *(u32 *)&POLY->u3 = *(u32 *)&PKT->tu3;
                        ADD_CLUT_ROWS(prim, ctx->dp >> ctx->dpShift);
                        gte_ldv0(&ctx->normals[PKT->n0]);
                        gte_ncds();
                        gte_strgb(&POLY->r0);
                        setcode(prim, ctx->primCode);
                        prim = SubmitPolyFT4((POLY_FT4 *)prim, ctx);
                    }
                    elem += sizeof(TMD_P_TF4);
                    packet += sizeof(TMD_P_TF4);
                    if (--count != 0)
                        goto loopH;
                    break;
#undef PKT
#undef POLY
                }

                case TMD_TYPE(GPU_COM_NTF4, TMD_FLAG_LGT): {
                    u8 *elem;
#define PKT ((TMD_P_TNF4 *)(elem - offsetof(TMD_P_TNF4, r0)))
#define POLY ((POLY_FT4 *)prim)

                    /* TMD_P_TNF4 -> POLY_FT4: unlit, its colour depth-cued. */
                    setPolyFT4(prim);
                    SetupPrimCode(prim, ctx);
                    elem = packet + offsetof(TMD_P_TNF4, r0);
                loopI:
                    if (ProjectQuadFace(prim, ctx, PKT->v0, PKT->v1, PKT->v2, PKT->v3,
                                        (void (*)(void *, s32))StoreSxyPolyFT4) == 0) {
                        *(u32 *)&POLY->u0 = *(u32 *)&PKT->tu0;
                        *(u32 *)&POLY->u1 = *(u32 *)&PKT->tu1;
                        *(u32 *)&POLY->u2 = *(u32 *)&PKT->tu2;
                        *(u32 *)&POLY->u3 = *(u32 *)&PKT->tu3;
                        ADD_CLUT_ROWS(prim, ctx->dp >> ctx->dpShift);
                        gte_ldrgb(&PKT->r0);
                        gte_dpcs();
                        gte_strgb(&POLY->r0);
                        setcode(prim, ctx->primCode);
                        prim = SubmitPolyFT4((POLY_FT4 *)prim, ctx);
                    }
                    elem += sizeof(TMD_P_TNF4);
                    packet += sizeof(TMD_P_TNF4);
                    if (--count != 0)
                        goto loopI;
                    break;
#undef PKT
#undef POLY
                }

                case TMD_TYPE(GPU_COM_NG3, TMD_FLAG_LGT): {
                    u8 *elem;
#define PKT ((TMD_P_NG3 *)(elem - offsetof(TMD_P_NG3, r0)))

                    /* TMD_P_NG3 -> POLY_G3: unlit, three colours depth-cued at once. */
                    setPolyG3(prim);
                    SetupPrimCode(prim, ctx);
                    elem = packet + offsetof(TMD_P_NG3, r0);
                loopJ:
                    if (ProjectTriFace(prim, ctx, PKT->v0, PKT->v1, PKT->v2, storeSxyG3) == 0) {
                        gte_ldrgb3c(&PKT->r0);
                        gte_dpct();
                        gte_strgb3_g3(prim);
                        setcode(prim, ctx->primCode);
                        prim = SubmitPolyG3((POLY_G3 *)prim, ctx);
                    }
                    elem += sizeof(TMD_P_NG3);
                    packet += sizeof(TMD_P_NG3);
                    if (--count != 0)
                        goto loopJ;
                    break;
#undef PKT
                }

                case TMD_TYPE(GPU_COM_NTG3, TMD_FLAG_LGT): {
                    u8 *elem;
#define PKT ((TMD_P_TNG3 *)(elem - offsetof(TMD_P_TNG3, r2)))
#define POLY ((POLY_GT3 *)prim)

                    /* TMD_P_TNG3 -> POLY_GT3: unlit, three colours depth-cued at once. */
                    setPolyGT3(prim);
                    SetupPrimCode(prim, ctx);
                    elem = packet + offsetof(TMD_P_TNG3, r2);
                loopK:
                    if (ProjectTriFace(prim, ctx, PKT->v0, PKT->v1, PKT->v2, StoreSxyPolyGT3) == 0) {
                        *(u32 *)&POLY->u0 = *(u32 *)&PKT->tu0;
                        *(u32 *)&POLY->u1 = *(u32 *)&PKT->tu1;
                        *(u32 *)&POLY->u2 = *(u32 *)&PKT->tu2;
                        ADD_CLUT_ROWS(prim, ctx->dp >> ctx->dpShift);
                        gte_ldrgb3(&((TMD_P_TNG3 *)packet)->r0, &((TMD_P_TNG3 *)packet)->r1, &PKT->r2);
                        gte_dpct();
                        gte_strgb3(&POLY->r0, &POLY->r1, &POLY->r2);
                        setcode(prim, ctx->primCode);
                        prim = SubmitPolyGT3((POLY_GT3 *)prim, ctx);
                    }
                    elem += sizeof(TMD_P_TNG3);
                    packet += sizeof(TMD_P_TNG3);
                    if (--count != 0)
                        goto loopK;
                    break;
#undef PKT
#undef POLY
                }

                case TMD_TYPE(GPU_COM_NG4, TMD_FLAG_LGT): {
                    u8 *elem;
#define PKT ((TMD_P_NG4 *)(elem - offsetof(TMD_P_NG4, r3)))
#define POLY ((POLY_G4 *)prim)

                    /* TMD_P_NG4 -> POLY_G4: unlit, three colours depth-cued at once
                     * and the fourth on its own. */
                    setPolyG4(prim);
                    SetupPrimCode(prim, ctx);
                    elem = packet + offsetof(TMD_P_NG4, r3);
                loopL:
                    if (ProjectQuadFace(prim, ctx, PKT->v0, PKT->v1, PKT->v2, PKT->v3,
                                        StoreSxyPolyG4) == 0) {
                        gte_ldrgb3c(&((TMD_P_NG4 *)packet)->r0);
                        gte_dpct();
                        gte_strgb3_g3(prim);
                        setcode(prim, ctx->primCode);
                        gte_ldrgb(&PKT->r3);
                        gte_dpcs();
                        gte_strgb(&POLY->r3);
                        prim = SubmitPolyG4((POLY_G4 *)prim, ctx);
                    }
                    elem += sizeof(TMD_P_NG4);
                    packet += sizeof(TMD_P_NG4);
                    if (--count != 0)
                        goto loopL;
                    break;
#undef PKT
#undef POLY
                }

                case TMD_TYPE(GPU_COM_NTG4, TMD_FLAG_LGT): {
                    u8 *elem;
#define PKT ((TMD_P_TNG4 *)(elem - offsetof(TMD_P_TNG4, r3)))
#define POLY ((POLY_GT4 *)prim)

                    /* TMD_P_TNG4 -> POLY_GT4: unlit, as TMD_P_NG4 plus four UVs. */
                    setPolyGT4(prim);
                    SetupPrimCode(prim, ctx);
                    elem = packet + offsetof(TMD_P_TNG4, r3);
                loopM:
                    if (ProjectQuadFace(prim, ctx, PKT->v0, PKT->v1, PKT->v2, PKT->v3,
                                        (void (*)(void *, s32))StoreSxyPolyGT4) == 0) {
                        *(u32 *)&POLY->u0 = *(u32 *)&PKT->tu0;
                        *(u32 *)&POLY->u1 = *(u32 *)&PKT->tu1;
                        *(u32 *)&POLY->u2 = *(u32 *)&PKT->tu2;
                        *(u32 *)&POLY->u3 = *(u32 *)&PKT->tu3;
                        ADD_CLUT_ROWS(prim, ctx->dp >> ctx->dpShift);
                        gte_ldrgb3(&((TMD_P_TNG4 *)packet)->r0, &((TMD_P_TNG4 *)packet)->r1,
                                   &((TMD_P_TNG4 *)packet)->r2);
                        gte_dpct();
                        gte_strgb3(&POLY->r0, &POLY->r1, &POLY->r2);
                        setcode(prim, ctx->primCode);
                        gte_ldrgb(&PKT->r3);
                        gte_dpcs();
                        gte_strgb(&POLY->r3);
                        prim = SubmitPolyGT4((POLY_GT4 *)prim, ctx);
                    }
                    elem += sizeof(TMD_P_TNG4);
                    packet += sizeof(TMD_P_TNG4);
                    if (--count != 0)
                        goto loopM;
                    break;
#undef PKT
#undef POLY
                }

                default:
                    return;
            }

            GsOUT_PACKET_P = prim;
        } while (packetsLeft != 0);
    }
}

/*
 * Finish the command byte of the primitive SortTmdObject has just given its
 * length and code, and cache both bytes in the context: the ABE
 * (semi-transparency) bit from the run's packet type, the TGE (texture
 * shading off) bit from sSortLightOff, the object's GsLOFF bit.
 * TransformAndCullPoly re-stamps the cached length onto every primitive.
 */
/* MATCHING: the length and code read back from the primitive separately;
 * one shared local compiles differently */
void SetupPrimCode(void *prim, PolyDrawCtx *ctx) {
    setSemiTrans(prim, ctx->semiTrans);
    setShadeTex(prim, sSortLightOff);

    ctx->primLen = getlen(prim);
    ctx->primCode = getcode(prim);
}

/*
 * Project one three-vertex face and say whether it survived.
 *
 * `idx0`..`idx2` index the vertex array the draw context holds; the three
 * resulting pointers are parked in the context's face slots and loaded into
 * the GTE. TransformAndCullPoly does the transform and the cull decision. On
 * success each vertex's screen Z goes into the sz of the three subdivision
 * vertices the context lists, `storeSxy` writes the screen XY into `prim` at
 * that primitive type's own offsets (one of the StoreSxyPoly** leaves at the
 * bottom of this file), and FlagLargePolyForDivide computes the screen
 * bounding box over the 3 vertices. Returns 0 drawn, 1 culled.
 *
 * `prim` is only ever handed straight through, so it stays void * here.
 */
s32 ProjectTriFace(void *prim, PolyDrawCtx *ctx, u16 idx0, u16 idx1, u16 idx2, void (*storeSxy)(void *)) {
    /* MATCHING: the slots are written through a plain pointer; as ctx->faceVtx[i]
     * the stores come out mixed in among the vertex address computations. */
    SVECTOR **vtx = ctx->faceVtx;

    vtx[0] = &ctx->vertices[idx0];
    vtx[1] = &ctx->vertices[idx1];
    vtx[2] = &ctx->vertices[idx2];

    gte_ldv3(vtx[0], vtx[1], vtx[2]);

    if (TransformAndCullPoly(prim, ctx) != 0) {
        goto fail;
    }

    {
        u_long *sz0 = &ctx->divVtx3[0]->sz;
        u_long *sz1 = &ctx->divVtx3[1]->sz;
        u_long *sz2 = &ctx->divVtx3[2]->sz;

        gte_stsz3(sz0, sz1, sz2);
    }
    storeSxy(prim);
    FlagLargePolyForDivide(ctx, 3);
    return 0;
fail:
    return 1;
}

/*
 * The four-vertex sibling of ProjectTriFace. The first three vertices go
 * through the same shared transform-and-cull; the fourth is transformed on
 * its own with a single rtps afterwards, which is why `storeSxy` is called
 * twice -- once with 1 to store the first three screen XYs, once with 0 to
 * store the fourth. Sort Zs go to four subdivision vertices, the fourth
 * vertex's screen XY is also cached in the context, and the bounding box is
 * computed over 4 vertices. Returns 0 drawn, 1 culled.
 */
s32 ProjectQuadFace(void *prim, PolyDrawCtx *ctx, u16 idx0, u16 idx1, u16 idx2, u16 idx3,
                    void (*storeSxy)(void *, s32)) {
    /* MATCHING: a plain pointer, as in ProjectTriFace. */
    SVECTOR **vtx = ctx->faceVtx;

    vtx[0] = &ctx->vertices[idx0];
    vtx[1] = &ctx->vertices[idx1];
    vtx[2] = &ctx->vertices[idx2];
    vtx[3] = &ctx->vertices[idx3];

    gte_ldv3(vtx[0], vtx[1], vtx[2]);

    if (TransformAndCullPoly(prim, ctx) != 0) {
        goto fail;
    }

    storeSxy(prim, 1);

    gte_ldv0(vtx[3]);
    gte_rtps();

    {
        u_long *sz0 = &ctx->divVtx4[0]->sz;
        u_long *sz1 = &ctx->divVtx4[1]->sz;
        u_long *sz2 = &ctx->divVtx4[2]->sz;
        u_long *sz3 = &ctx->divVtx4[3]->sz;

        gte_stsz4(sz0, sz1, sz2, sz3);
    }

    storeSxy(prim, 0);

    gte_stsxy2(&ctx->sxy[3]);

    FlagLargePolyForDivide(ctx, 4);
    return 0;
fail:
    return 1;
}

/*
 * Perspective-transform the three vertices the caller already loaded into
 * the GTE (gte_ldv3), then cull: bail if the FLAG register shows anything
 * but clean or SZ3/OTZ-saturated (which marks the face for subdivision), if
 * the polygon is back-facing (nclip <= 0), or if the depth-cue factor has
 * reached ONE. Otherwise average the Z, cache the screen coordinates and
 * compute the OT slot. Returns 0 on success, 1 when culled.
 */
s32 TransformAndCullPoly(void *primIn, void *ctxIn) {
    PolyDrawCtx *ctx = ctxIn;

    ctx->divide = 0;
    gte_rtpt();
    setlen(primIn, ctx->primLen);
    gte_stflg(&ctx->flag);
    if (ctx->flag != 0) {
        if (ctx->flag != GTE_FLAG_SZ3_OTZ_SAT) {
            return 1;
        }
        ctx->divide = 1;
    }
    gte_nclip();
    gte_stopz(&ctx->opz);
    if (ctx->opz <= 0) {
        return 1;
    }
    gte_stdp(&ctx->dp);
    if (ctx->dp >= ONE) {
        return 1;
    }
    gte_avsz3();
    gte_stotz(&ctx->otz);
    gte_stsxy3(&ctx->sxy[0], &ctx->sxy[1], &ctx->sxy[2]);
    ctx->otSlot = &ctx->otBase[ctx->otz >> ctx->otShift];
    return 0;
}

/* The screen-XY store callbacks ProjectTri/QuadFace invoke, one per
 * primitive type: each writes the GTE's SXY FIFO into that POLY_xx's own
 * vertex fields (StoreSxyPolyFT4 and StoreSxyPolyGT4 are with the submit
 * wrappers below).
 * POLY_F3: xy0/xy1/xy2 at +0x8/+0xC/+0x10. */
void StoreSxyPolyF3(void *dst) {
    gte_stsxy3_f3(dst);
}

/* POLY_G3: +0x8/+0x10/+0x18 (per-vertex RGB between the XYs). */
void StoreSxyPolyG3(void *dst) {
    gte_stsxy3_g3(dst);
}

/* POLY_FT3: +0x8/+0x10/+0x18, the same offsets as POLY_G3 (a UV word where
 * POLY_G3 has an RGB word), but a function of its own; the primitive at
 * each call site says which is which. */
void StoreSxyPolyFT3(void *dst) {
    gte_stsxy3_ft3(dst);
}

/* POLY_GT3: +0x8/+0x14/+0x20 (RGB and UV between the XYs). */
void StoreSxyPolyGT3(void *dst) {
    gte_stsxy3_gt3(dst);
}

/*
 * POLY_F4: xy0/xy1/xy2 at +0x8/+0xC/+0x10, xy3 at +0x14. ProjectQuadFace
 * calls this twice -- storeFirst3 = 1 for the three vertices the shared
 * transform produced, then 0 for the fourth vertex's own rtps result.
 */
/* MATCHING: `xy3` is computed before the test, on both paths */
void StoreSxyPolyF4(void *dst, s32 storeFirst3) {
    short *xy3 = &((POLY_F4 *)dst)->x3;

    if (storeFirst3) {
        gte_stsxy3_f4(dst);
    } else {
        gte_stsxy2(xy3);
    }
}

/* POLY_G4: xy0/xy1/xy2 at +0x8/+0x10/+0x18, xy3 at +0x20. Same two-call
 * protocol as StoreSxyPolyF4. */
void StoreSxyPolyG4(void *dst, s32 storeFirst3) {
    short *xy3 = &((POLY_G4 *)dst)->x3;

    if (storeFirst3) {
        gte_stsxy3_g4(dst);
    } else {
        gte_stsxy2(xy3);
    }
}

/* The widest or tallest screen extent, in pixels, a face may have and still
 * be linked into the OT as one primitive; FlagLargePolyForDivide sends
 * anything larger through Sony's RCpoly* subdivision. 256 is also the width
 * of one texture page. */
#define MAX_UNDIVIDED_SPAN 256

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
 */
/* MATCHING: the divide arm comes first and returns; with the addPrim arm first
 * the two paths come out in the other order */
void *SubmitPolyF3(POLY_F3 *prim, PolyDrawCtx *ctx) {
    if (ctx->divide != 0) {
        FillDivPolygonHeader(sDivPolygon3, ctx, (CVECTOR *)&prim->r0, 0, 0, 0);
        FillRVectors3(ctx->divVtx3, ctx->faceVtx, (DVECTOR *)&prim->x0, (DVECTOR *)&prim->x1,
                      (DVECTOR *)&prim->x2);
        return RCpolyF3(prim, (DIVPOLYGON3 *)sDivPolygon3);
    }
    addPrim(ctx->otSlot, prim);
    return (u_long *)(prim + 1);
}

void *SubmitPolyG3(POLY_G3 *prim, PolyDrawCtx *ctx) {
    if (ctx->divide != 0) {
        FillDivPolygonHeader(sDivPolygon3, ctx, (CVECTOR *)&prim->r0, 0, 0, 0);
        FillRVectors3(ctx->divVtx3, ctx->faceVtx, (DVECTOR *)&prim->x0, (DVECTOR *)&prim->x1,
                      (DVECTOR *)&prim->x2);

        ctx->divVtx3[0]->pad = prim->pad1;
        ctx->divVtx3[1]->pad = prim->pad1;
        ctx->divVtx3[2]->pad = prim->pad2;

        ctx->divVtx3[0]->c = *(CVECTOR *)&prim->r0;
        ctx->divVtx3[1]->c = *(CVECTOR *)&prim->r1;
        ctx->divVtx3[2]->c = *(CVECTOR *)&prim->r2;

        return RCpolyG3(prim, (DIVPOLYGON3 *)sDivPolygon3);
    }
    addPrim(ctx->otSlot, prim);
    return (u_long *)(prim + 1);
}

void *SubmitPolyFT3(POLY_FT3 *prim, PolyDrawCtx *ctx) {
    if (ctx->divide != 0) {
        FillDivPolygonHeader(sDivPolygon3, ctx, (CVECTOR *)&prim->r0, 1, prim->clut, prim->tpage);
        FillRVectors3(ctx->divVtx3, ctx->faceVtx, (DVECTOR *)&prim->x0, (DVECTOR *)&prim->x1,
                      (DVECTOR *)&prim->x2);

        ctx->divVtx3[0]->pad = prim->pad1;
        ctx->divVtx3[1]->pad = prim->pad1;
        ctx->divVtx3[2]->pad = prim->pad1;
        *(u_short *)ctx->divVtx3[0]->uv = *(u_short *)&prim->u0;
        *(u_short *)ctx->divVtx3[1]->uv = *(u_short *)&prim->u1;
        *(u_short *)ctx->divVtx3[2]->uv = *(u_short *)&prim->u2;

        return RCpolyFT3(prim, (DIVPOLYGON3 *)sDivPolygon3);
    }
    addPrim(ctx->otSlot, prim);
    return (u_long *)(prim + 1);
}

void *SubmitPolyF4(POLY_F4 *prim, PolyDrawCtx *ctx) {
    if (ctx->divide != 0) {
        FillDivPolygonHeader(sDivPolygon4, ctx, (CVECTOR *)&prim->r0, 0, 0, 0);
        FillRVectors4(ctx->divVtx4, ctx->faceVtx, (DVECTOR *)&prim->x0, (DVECTOR *)&prim->x1,
                      (DVECTOR *)&prim->x2, (DVECTOR *)&prim->x3);
        return RCpolyF4(prim, (DIVPOLYGON4 *)sDivPolygon4);
    }
    addPrim(ctx->otSlot, prim);
    return (u_long *)(prim + 1);
}

void *SubmitPolyG4(POLY_G4 *prim, PolyDrawCtx *ctx) {
    if (ctx->divide != 0) {
        FillDivPolygonHeader(sDivPolygon4, ctx, (CVECTOR *)&prim->r0, 0, 0, 0);
        FillRVectors4(ctx->divVtx4, ctx->faceVtx, (DVECTOR *)&prim->x0, (DVECTOR *)&prim->x1,
                      (DVECTOR *)&prim->x2, (DVECTOR *)&prim->x3);

        ctx->divVtx4[0]->pad = prim->pad1;
        ctx->divVtx4[1]->pad = prim->pad1;
        ctx->divVtx4[2]->pad = prim->pad2;
        ctx->divVtx4[3]->pad = prim->pad3;

        ctx->divVtx4[0]->c = *(CVECTOR *)&prim->r0;
        ctx->divVtx4[1]->c = *(CVECTOR *)&prim->r1;
        ctx->divVtx4[2]->c = *(CVECTOR *)&prim->r2;
        ctx->divVtx4[3]->c = *(CVECTOR *)&prim->r3;

        return RCpolyG4(prim, (DIVPOLYGON4 *)sDivPolygon4);
    }
    addPrim(ctx->otSlot, prim);
    return (u_long *)(prim + 1);
}

void *SubmitPolyFT4(POLY_FT4 *prim, PolyDrawCtx *ctx) {
    if (ctx->divide != 0) {
        FillDivPolygonHeader(sDivPolygon4, ctx, (CVECTOR *)&prim->r0, 1, prim->clut, prim->tpage);
        FillRVectors4(ctx->divVtx4, ctx->faceVtx, (DVECTOR *)&prim->x0, (DVECTOR *)&prim->x1,
                      (DVECTOR *)&prim->x2, (DVECTOR *)&prim->x3);

        ctx->divVtx4[0]->pad = prim->pad1;
        ctx->divVtx4[1]->pad = prim->pad1;
        ctx->divVtx4[2]->pad = prim->pad1;
        ctx->divVtx4[3]->pad = prim->pad1;
        *(u_short *)ctx->divVtx4[0]->uv = *(u_short *)&prim->u0;
        *(u_short *)ctx->divVtx4[1]->uv = *(u_short *)&prim->u1;
        *(u_short *)ctx->divVtx4[2]->uv = *(u_short *)&prim->u2;
        *(u_short *)ctx->divVtx4[3]->uv = *(u_short *)&prim->u3;

        return RCpolyFT4(prim, (DIVPOLYGON4 *)sDivPolygon4);
    }
    addPrim(ctx->otSlot, prim);
    return (u_long *)(prim + 1);
}

void *SubmitPolyGT3(POLY_GT3 *prim, PolyDrawCtx *ctx) {
    if (ctx->divide != 0) {
        FillDivPolygonHeader(sDivPolygon3, ctx, (CVECTOR *)&prim->r0, 1, prim->clut, prim->tpage);
        FillRVectors3(ctx->divVtx3, ctx->faceVtx, (DVECTOR *)&prim->x0, (DVECTOR *)&prim->x1,
                      (DVECTOR *)&prim->x2);

        ctx->divVtx3[0]->pad = prim->pad2;
        ctx->divVtx3[1]->pad = prim->pad2;
        ctx->divVtx3[2]->pad = prim->pad2;

        ctx->divVtx3[0]->c = *(CVECTOR *)&prim->r0;
        ctx->divVtx3[1]->c = *(CVECTOR *)&prim->r1;
        ctx->divVtx3[2]->c = *(CVECTOR *)&prim->r2;

        *(u_short *)ctx->divVtx3[0]->uv = *(u_short *)&prim->u0;
        *(u_short *)ctx->divVtx3[1]->uv = *(u_short *)&prim->u1;
        *(u_short *)ctx->divVtx3[2]->uv = *(u_short *)&prim->u2;

        return RCpolyGT3(prim, (DIVPOLYGON3 *)sDivPolygon3);
    }
    addPrim(ctx->otSlot, prim);
    return (u_long *)(prim + 1);
}

void *SubmitPolyGT4(POLY_GT4 *prim, PolyDrawCtx *ctx) {
    if (ctx->divide != 0) {
        FillDivPolygonHeader(sDivPolygon4, ctx, (CVECTOR *)&prim->r0, 1, prim->clut, prim->tpage);
        FillRVectors4(ctx->divVtx4, ctx->faceVtx, (DVECTOR *)&prim->x0, (DVECTOR *)&prim->x1,
                      (DVECTOR *)&prim->x2, (DVECTOR *)&prim->x3);

        ctx->divVtx4[0]->pad = prim->pad2;
        ctx->divVtx4[1]->pad = prim->pad2;
        ctx->divVtx4[2]->pad = prim->pad3;
        ctx->divVtx4[3]->pad = prim->pad3;

        ctx->divVtx4[0]->c = *(CVECTOR *)&prim->r0;
        ctx->divVtx4[1]->c = *(CVECTOR *)&prim->r1;
        ctx->divVtx4[2]->c = *(CVECTOR *)&prim->r2;
        ctx->divVtx4[3]->c = *(CVECTOR *)&prim->r3;

        *(u_short *)ctx->divVtx4[0]->uv = *(u_short *)&prim->u0;
        *(u_short *)ctx->divVtx4[1]->uv = *(u_short *)&prim->u1;
        *(u_short *)ctx->divVtx4[2]->uv = *(u_short *)&prim->u2;
        *(u_short *)ctx->divVtx4[3]->uv = *(u_short *)&prim->u3;

        return RCpolyGT4(prim, (DIVPOLYGON4 *)sDivPolygon4);
    }
    addPrim(ctx->otSlot, prim);
    return (u_long *)(prim + 1);
}

/*
 * Point a draw context's RVECTOR list (`vtxPtrs`, its divVtx3 or divVtx4)
 * and the DIVPOLYGON's own first recursion level (cr[0].r0...) at that
 * DIVPOLYGON's vertex records r0, r1, ... `nverts` is 3 for a DIVPOLYGON3,
 * 4 for a DIVPOLYGON4; SortTmdObject calls it for both, once per object.
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
 * the last vertex never enters the box.
 */
void FlagLargePolyForDivide(void *ctxIn, s32 count) {
    PolyDrawCtx *ctx = ctxIn;
    short *xp, *yp, *end;

    ctx->bboxMax = ctx->sxy[0];
    ctx->bboxMin = ctx->bboxMax;

    xp = &ctx->sxy[1].vx;
    /* MATCHING: this is &ctx->sxy[count - 1].vx, but that spelling folds the
     * constant into the index before adding ctx, and retail adds ctx first. */
    end = (short *)((u8 *)ctx + count * sizeof(DVECTOR) + (offsetof(PolyDrawCtx, sxy) - sizeof(DVECTOR)));

    /* MATCHING: a guarded do/while with yp set inside the guard; setting yp
     * before the test orders the set-up differently around the branch. */
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

/* SetNdivOverride's: when set, sNdivOverride replaces sSortNdiv. */
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
     * after it; a ternary or a store per arm is a word longer. */
    if (sNdivOverrideSet) {
        ndiv = sNdivOverride;
    } else {
        ndiv = sSortNdiv;
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
    divp->ot = (u_long *)ctx->otSlot; /* DIVPOLYGON keeps it as a u_long * */
}

/* Give three RVECTORs their model-space vertex and screen XY. */
/* MATCHING: whole-struct assignments, not member copies */
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
