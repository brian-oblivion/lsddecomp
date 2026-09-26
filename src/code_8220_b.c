/*
 * code_8220_b -- the tail of the BasicClass block, then the game's own
 * per-face polygon renderer. Two unrelated halves, split at SortTmdObject.
 *
 * 0x80018288..0x80018458 finishes BasicClass, the hand-rolled base class
 * whose framework and 14-slot method table live in code_8220.c and
 * include/code_8220.h: the two list primitives BasicClass's own methods
 * call (FreeBasicClassList, GetNextBasicClass), the array release helper,
 * the vtable accessor, the two notification slots (BasicClass__NotifyParents
 * at +0x030 and BasicClass__OnNotify at +0x038) and the empty +0x034 hook,
 * plus the accessor pair for the pool allocator's re-entrancy flag.
 *
 * 0x80018464..0x8001974C is the renderer. SortTmdObject (the unit's one
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
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "code_8220.h"
#include "gte.h"

void FreeBasicClassList(BasicClassListNode **head) {
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
void BasicClass__NotifyParents(BasicClass *self, s32 event) {
    BasicClassListNode *cursor = self->parentRefs;
    BasicClass *parent;

    for (GetNextBasicClass(&parent, &cursor); parent != NULL; GetNextBasicClass(&parent, &cursor)) {
        parent->methods->onNotify(parent, self, event);
    }
}

/* BasicClassMethods slot +0x034, and deliberately still a placeholder name.
 * The body is empty, and of the 60 method tables tools/classtable.py finds,
 * 58 have this exact address in slot +0x034; the two that differ
 * (D_8006C0F8, whose header word is a pointer and which is probably a
 * mis-detected table start, and gStyleCueCallbacks, a wholly independent 14-slot
 * class that overrides every slot) are not BasicClass-derived. Nothing in
 * the game overrides it, so nothing establishes what it is for. */
void BasicClass__func_18350(void) {}

/* BasicClassMethods slot +0x038, the receiving half of NotifyParents:
 * `sender` is telling `self` that `event` happened. The base class treats
 * event 1 as "sender is going away" and drops it from its own children.
 * Subclasses override this and forward to the base first -- Class6B5CC__OnNotify
 * (code_d294) then dispatches on the sender's class tag, Class65650__OnNotify
 * (code_55dd4) then checks the sender's tag and the same event == 1 -- so
 * `event` is a general notification code, not a boolean. */
void BasicClass__OnNotify(BasicClass *self, void *sender, s32 event) {
    if (event == 1) {
        self->methods->removeChild(self, (BasicClass *)sender);
    }
}

BasicClassMethods *Get_vtable_BasicClass(void) {
    return &D_8006B58C;
}

void GetNextBasicClass(BasicClass **outValue, BasicClassListNode **cursor) {
    if (*cursor != NULL) {
        *outValue = (*cursor)->value;
        *cursor = (*cursor)->next;
    } else {
        *outValue = NULL;
    }
}

void ReleaseBasicClassArray(BasicClass **array, s32 count) {
    if (count-- > 0) {
        do {
            *array = (BasicClass *)(*array)->methods->release(*array);
            array++;
        } while (count-- > 0);
    }
}

void SetBMemPMgrBusy(s32 busy) {
    gBMemPMgrBusy = busy;
}

s32 GetBMemPMgrBusy(void) {
    return gBMemPMgrBusy;
}

/* Globals this renderer publishes for SetupPrimCode and the code_8220_c
 * submit wrappers to read back. D_8008E248 is already in code_8220.h; the
 * other four are this unit's own view and stay local per CLAUDE.md's
 * cross-unit-declaration rule. */
extern s32 D_80090C18;
extern s32 gSortUseGlobalLightMode;
extern s32 gSortLightMode;

/* Rgb8 makes the tint a 3-byte block move: `la`, three `lb`, three `sb`. */
typedef struct {
    s8 r, g, b;
} Rgb8;

/*
 * The per-object draw context SortTmdObject builds in the PS1 scratchpad
 * (its caller passes 0x1F800000) and hands to every function below. Only
 * the fields this unit touches are named; +0x070..+0x077 are the screen
 * bounding box UpdatePolyBBoxAndCull (code_8220_c) keeps.
 *
 * The three SXY words are deliberately separate fields rather than an array:
 * retail stores them through three independently computed addresses, which
 * is gte_stsxy3()'s three-pointer form.
 */
typedef struct PolyDrawCtx {
    /* +0x000 */ GsOT_TAG *otBase;  /* the GsOT's org */
    /* +0x004 */ s32 otShift;       /* otz >> otShift indexes otBase */
    /* +0x008 */ s32 unk8;          /* set to 10 per object; no reader in carved code */
    /* +0x00C */ SVECTOR *vertices; /* the TMD object's vertex array */
    /* +0x010 */ SVECTOR *normals;  /* the TMD object's normal array */
    /* +0x014 */ u8 primLen;        /* SetupPrimCode's cached P_TAG length byte */
    /* +0x015 */ u8 primCode;       /* ... and finished GPU command byte */
    u8 pad016[0x018 - 0x016];
    /* +0x018 */ u32 packetType;   /* the current group's TMD mode/flag word */
    /* +0x01C */ s32 semiTrans;    /* the group's ABE bit */
    /* +0x020 */ s32 otz;          /* avsz3 result */
    /* +0x024 */ s32 dp;           /* IR0, the depth-cue factor */
    /* +0x028 */ s32 opz;          /* nclip result (MAC0) */
    /* +0x02C */ s32 dpShift;      /* dp >> dpShift is the CLUT row offset */
    /* +0x030 */ GsOT_TAG *otSlot; /* &otBase[otz >> otShift] */
    /* +0x034 */ Rgb8 faceColor;   /* gTexturedFaceColor's copy */
    u8 pad037[0x038 - 0x037];
    /* +0x038 */ MATRIX savedRotMatrix;
    u8 pad058[0x05C - 0x058];
    /* +0x05C */ s32 flag; /* GTE FLAG */
    /* +0x060 */ s32 sxy0;
    /* +0x064 */ s32 sxy1;
    /* +0x068 */ s32 sxy2;
    /* +0x06C */ s32 sxy3; /* a quad's fourth vertex */
    u8 pad070[0x078 - 0x070];
    /* +0x078 */ s32 divide; /* set when the face must go through RCpoly* subdivision */
    u8 pad07C[0x088 - 0x07C];
    /* +0x088 */ RVECTOR *divVtx3[3]; /* gPolySubmitTableTri's r0..r2 */
    /* +0x094 */ RVECTOR *divVtx4[4]; /* gPolySubmitTableQuad's r0..r3 */
    /* +0x0A4 */ SVECTOR *faceVtx[4]; /* the current face's vertices */
} PolyDrawCtx;

/* The first word of a group's first TMD packet. Sony's TMD_P_* structs
 * spell it as four bytes (out, in, dummy, cd: olen, ilen, flag, mode); this
 * renderer reads the first two as the group's packet count and the last two
 * as one flag | mode << 8 word. */
typedef struct TmdGroupHeader {
    u16 count;
    u16 type;
} TmdGroupHeader;

/* A TMD packet's type as SortTmdObject switches on it: the mode byte
 * (Sony's GPU_COM_* codes, <libgs.h>) above the flag byte. */
#define TMD_TYPE(mode, flag) ((mode) << 8 | (flag))
#define TMD_FLAG_LGT 0x01    /* flag: light source calculation off */
#define TMD_TYPE_MASK 0xFD07 /* mode less its ABE bit 0x02; flag's LGT, FCE, GRD */

/* GTE FLAG bit 18: SZ3 or OTZ saturated. The one FLAG state
 * TransformAndCullPoly keeps a polygon for, routed to subdivision. */
#define GTE_FLAG_SZ3_OTZ_SAT 0x40000

/* GsDOBJ2 keeps its TMD object as a u_long *. */
#define OBJ_TMD(obj) ((struct TMD_STRUCT *)(obj)->tmd)

/* Advance a primitive's CLUT by `rows` palette rows. A statement macro, and
 * its do/while(0) is part of the bytes: the per-element loops below are goto
 * loops (see the function comment), which drops one loop-depth level from
 * flow's reference weighting, and this construct puts it back for the update
 * alone -- without it local-alloc hands the loaded CLUT, not the shifted
 * depth, $v0 at all six sites. */
/* clang-format off */
#define ADD_CLUT_ROWS(p, rows) \
    do { \
        ((POLY_FT3 *)(p))->clut += (rows) << 6; \
    } while (0)
/* clang-format on */
extern Rgb8 gTexturedFaceColor;

extern void InitVtxRecordPtrs(void *dst, void *table, s32 count);
extern void StoreSxyPolyFT4(void *dst, s32 storeFirst3);
extern void StoreSxyPolyGT4(void *dst, s32 storeFirst3);

/*
 * The eight submit wrappers in code_8220_c, which are still INCLUDE_ASM there
 * and declared `void`. That is provably a placeholder: every one of them is a
 * tail call whose last instruction before its epilogue is `jal RCpolyXX` with
 * no intervening store to $v0, so Sony's return value falls straight out --
 * the standard `p = RCpolyF3(p);` work-buffer-advance idiom. This function
 * consumes exactly that value, so this file declares its own view rather than
 * importing the stale one (round 50's finding; see the report).
 */
extern void *SubmitPolyF3(void *prim, void *ctx);
extern void *SubmitPolyG3(void *prim, void *ctx);
extern void *SubmitPolyFT3(void *prim, void *ctx);
extern void *SubmitPolyF4(void *prim, void *ctx);
extern void *SubmitPolyG4(void *prim, void *ctx);
extern void *SubmitPolyFT4(void *prim, void *ctx);
extern void *SubmitPolyGT3(void *prim, void *ctx);
extern void *SubmitPolyGT4(void *prim, void *ctx);

/* Defined below, in ROM order. Forward-declared because this function comes
 * first in the segment and calls all of them. */
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

/*
 * Walk one model's face groups and emit a GPU primitive per surviving face.
 *
 * Every GTE access goes through include/gte.h. The RGB store macros are the
 * one-pointer-at-offset-0 Psy-Q forms, which is why `gte_strgb(&POLY->r0)`
 * and not `swc2 $22, 0x4(prim)`: the addiu that materialises the sum is part
 * of retail.
 *
 * The thirteen per-element loops are goto loops, not do/while: GCC 2.6.3's
 * loop optimiser only runs on syntactic loops, and there it strength-reduces
 * `elem`'s constant-offset uses into a second induction variable (the
 * gte_ldrgb "r" operand keeps the original alive), costing a setup `addiu`,
 * an increment per iteration and an eighth saved register -- +28 words in
 * all. `ctx` is assigned after the early return for the same reason retail
 * copies a3 through a1 into s2 there.
 */
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
    InitVtxRecordPtrs(ctx->divVtx3, gPolySubmitTableTri, 3);
    InitVtxRecordPtrs(ctx->divVtx4, gPolySubmitTableQuad, 4);

    packetsLeft = OBJ_TMD(obj)->primn;
    packet = (u8 *)OBJ_TMD(obj)->primtop;
    ctx->vertices = (SVECTOR *)OBJ_TMD(obj)->vertop;
    ctx->normals = (SVECTOR *)OBJ_TMD(obj)->nortop;

    /* When the object's coordinate system has a parent, save the GTE's
     * current rotation matrix into the context, install the parent's world
     * matrix, run each of the three columns of the object's own through it,
     * and put the saved matrix back. */
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

    /* Four reads of the attribute word, not one cached copy: CSE treats each
     * global store as a possible alias and keeps all four `lw`s, while the
     * scheduler (which does know a struct field from a fixed scalar) then
     * hoists them together above the stores. The unk8 store must be a struct
     * field written after them, so that it waits behind the loads. */
    D_80090C18 = (obj->attribute >> 9) & 0x7;
    D_8008E248 = (obj->attribute >> 6) & 0x1;
    gSortUseGlobalLightMode = (obj->attribute >> 5) & 0x1;
    gSortLightMode = (obj->attribute >> 3) & 0x3;
    ctx->unk8 = 10;

    ctx->faceColor = gTexturedFaceColor;

    if ((gSortUseGlobalLightMode != 0 && GsLIGHT_MODE != 0) || gSortLightMode != 0) {
        dpShift = 9;
    } else {
        dpShift = 16;
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
            ctx->semiTrans = (*(u32 *)packet >> 25) & 0x1;
            packetsLeft -= count;

            /* Thirteen Psy-Q primitive flavours, one case each, in ascending
             * tag order. GCC 2.6.3 expands a switch over sparse values as a
             * balanced binary search -- `== median`, then `< median + 1` and
             * recurse -- which is where retail's sltiu/beq ladder comes from;
             * the case bodies then follow in source order, which is why they
             * sit at ascending addresses in ascending tag order. */
            switch (ctx->packetType) {
                case TMD_TYPE(GPU_COM_F3, 0): {
                    u8 *elem;
#define PKT ((TMD_P_F3 *)(elem - offsetof(TMD_P_F3, r0)))
#define POLY ((POLY_F3 *)prim)

                    /* A: POLY_F3, opaque */
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
                        prim = SubmitPolyF3(prim, ctx);
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

                    /* B: POLY_G3, opaque -- one ncds per vertex colour */
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
                        prim = SubmitPolyG3(prim, ctx);
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

                    /* C: POLY_F3, depth-cued */
                    setPolyF3(prim);
                    SetupPrimCode(prim, ctx);
                    elem = packet + offsetof(TMD_P_NF3, r0);
                loopC:
                    if (ProjectTriFace(prim, ctx, PKT->v0, PKT->v1, PKT->v2, StoreSxyPolyF3) == 0) {
                        gte_ldrgb(&PKT->r0);
                        gte_dpcs();
                        gte_strgb(&POLY->r0);
                        setcode(prim, ctx->primCode);
                        prim = SubmitPolyF3(prim, ctx);
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

                    /* D: POLY_FT3, opaque -- one constant colour for the whole
                     * group, loaded once before the loop. */
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
                        prim = SubmitPolyFT3(prim, ctx);
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

                    /* E: POLY_FT3, depth-cued */
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
                        prim = SubmitPolyFT3(prim, ctx);
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

                    /* F: POLY_F4, opaque */
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
                        prim = SubmitPolyF4(prim, ctx);
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

                    /* G: POLY_F4, depth-cued */
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
                        prim = SubmitPolyF4(prim, ctx);
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

                    /* H: POLY_FT4, opaque */
                    setPolyFT4(prim);
                    SetupPrimCode(prim, ctx);
                    gte_ldrgb(&ctx->faceColor);
                    elem = packet + offsetof(TMD_P_TF4, n0);
                loopH:
                    if (ProjectQuadFace(prim, ctx, PKT->v0, PKT->v1, PKT->v2, PKT->v3,
                                        StoreSxyPolyFT4) == 0) {
                        *(u32 *)&POLY->u0 = *(u32 *)&PKT->tu0;
                        *(u32 *)&POLY->u1 = *(u32 *)&PKT->tu1;
                        *(u32 *)&POLY->u2 = *(u32 *)&PKT->tu2;
                        *(u32 *)&POLY->u3 = *(u32 *)&PKT->tu3;
                        ADD_CLUT_ROWS(prim, ctx->dp >> ctx->dpShift);
                        gte_ldv0(&ctx->normals[PKT->n0]);
                        gte_ncds();
                        gte_strgb(&POLY->r0);
                        setcode(prim, ctx->primCode);
                        prim = SubmitPolyFT4(prim, ctx);
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

                    /* I: POLY_FT4, depth-cued */
                    setPolyFT4(prim);
                    SetupPrimCode(prim, ctx);
                    elem = packet + offsetof(TMD_P_TNF4, r0);
                loopI:
                    if (ProjectQuadFace(prim, ctx, PKT->v0, PKT->v1, PKT->v2, PKT->v3,
                                        StoreSxyPolyFT4) == 0) {
                        *(u32 *)&POLY->u0 = *(u32 *)&PKT->tu0;
                        *(u32 *)&POLY->u1 = *(u32 *)&PKT->tu1;
                        *(u32 *)&POLY->u2 = *(u32 *)&PKT->tu2;
                        *(u32 *)&POLY->u3 = *(u32 *)&PKT->tu3;
                        ADD_CLUT_ROWS(prim, ctx->dp >> ctx->dpShift);
                        gte_ldrgb(&PKT->r0);
                        gte_dpcs();
                        gte_strgb(&POLY->r0);
                        setcode(prim, ctx->primCode);
                        prim = SubmitPolyFT4(prim, ctx);
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

                    /* J: POLY_G3, depth-cued -- three colours, one dpct */
                    setPolyG3(prim);
                    SetupPrimCode(prim, ctx);
                    elem = packet + offsetof(TMD_P_NG3, r0);
                loopJ:
                    if (ProjectTriFace(prim, ctx, PKT->v0, PKT->v1, PKT->v2, storeSxyG3) == 0) {
                        gte_ldrgb3c(&PKT->r0);
                        gte_dpct();
                        gte_strgb3_g3(prim);
                        setcode(prim, ctx->primCode);
                        prim = SubmitPolyG3(prim, ctx);
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

                    /* K: POLY_GT3, depth-cued */
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
                        prim = SubmitPolyGT3(prim, ctx);
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

                    /* L: POLY_G4, depth-cued -- three colours by dpct, the
                     * fourth by a second dpcs. */
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
                        prim = SubmitPolyG4(prim, ctx);
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

                    /* M: POLY_GT4, depth-cued -- the widest element, 0x2C bytes,
                     * all four UVs and all four colours. */
                    setPolyGT4(prim);
                    SetupPrimCode(prim, ctx);
                    elem = packet + offsetof(TMD_P_TNG4, r3);
                loopM:
                    if (ProjectQuadFace(prim, ctx, PKT->v0, PKT->v1, PKT->v2, PKT->v3,
                                        StoreSxyPolyGT4) == 0) {
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
                        prim = SubmitPolyGT4(prim, ctx);
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
 * Finish the GPU command byte of the POLY_xx primitive `prim`, then cache it
 * in the draw context. `prim[3]` is the P_TAG length byte and `prim[7]` is
 * the GPU command byte; SortTmdObject writes the (len, code) pair for the
 * primitive type immediately before every one of its 13 calls to this, and
 * all eight pairs it uses are Sony's exactly -- (4, 0x20) POLY_F3, (6, 0x30)
 * POLY_G3, (7, 0x24) POLY_FT3, (5, 0x28) POLY_F4, (9, 0x2C) POLY_FT4,
 * (9, 0x34) POLY_GT3, (8, 0x38) POLY_G4, (12, 0x3C) POLY_GT4.
 *
 * Bit 0x2 of the command byte is the GPU's ABE (semi-transparency) bit and
 * is taken from the context's own flag; bit 0x1 is the shade-texture
 * bit (Psy-Q SetShadeTex) and is taken from the global D_8008E248, which
 * SortTmdObject sets from bit 6 of the object's flags word. The two updates
 * are deliberately independent statements -- see this function's match
 * report, factoring them through one local costs the match.
 *
 * The length byte and the finished command byte are then cached in the
 * context; TransformAndCullPoly re-stamps the length byte onto every
 * primitive it processes.
 */
void SetupPrimCode(void *prim, PolyDrawCtx *ctx) {
    setSemiTrans(prim, ctx->semiTrans);
    setShadeTex(prim, D_8008E248);

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
 * bottom of this file), and UpdatePolyBBoxAndCull computes the screen
 * bounding box over the 3 vertices. Returns 0 drawn, 1 culled.
 *
 * `prim` is only ever handed straight through, so it stays void * here.
 */
s32 ProjectTriFace(void *prim, PolyDrawCtx *ctx, u16 idx0, u16 idx1, u16 idx2, void (*storeSxy)(void *)) {
    /* MATCHING: the slots go through a plain pointer; as ctx->faceVtx[i]
     * struct stores the scheduler interleaves them with the loads. */
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
    UpdatePolyBBoxAndCull(ctx, 3);
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

    gte_stsxy2(&ctx->sxy3);

    UpdatePolyBBoxAndCull(ctx, 4);
    return 0;
fail:
    return 1;
}

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
    gte_stsxy3(&ctx->sxy0, &ctx->sxy1, &ctx->sxy2);
    ctx->otSlot = &ctx->otBase[ctx->otz >> ctx->otShift];
    return 0;
}

/* The six screen-XY store callbacks ProjectTri/QuadFace invoke, one per
 * Psy-Q primitive type: each writes the GTE's SXY FIFO into that POLY_xx's
 * own vertex offsets. POLY_FT4 and POLY_GT4's equivalents are StoreSxyPolyFT4
 * and StoreSxyPolyGT4 in code_8220_c. POLY_F3: xy0/xy1/xy2 at +0x8/+0xC/+0x10. */
void StoreSxyPolyF3(void *dst) {
    gte_stsxy3_f3(dst);
}

/* POLY_G3: +0x8/+0x10/+0x18 (per-vertex RGB between the XYs). */
void StoreSxyPolyG3(void *dst) {
    gte_stsxy3_g3(dst);
}

/* POLY_FT3: +0x8/+0x10/+0x18 -- byte-identical to the POLY_G3 store, since
 * POLY_G3's per-vertex RGB and POLY_FT3's per-vertex UV are both 4 bytes.
 * Retail keeps them as two separate functions and so does this file; which
 * is which is settled by the (len, code) pair at each one's call site, not
 * by the offsets. */
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
 * `xy3` must be a real unconditionally-computed pointer, not a 0x14(%0)
 * offset inside the asm: see this function's match report.
 */
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
