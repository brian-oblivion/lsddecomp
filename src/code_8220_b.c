#include "common.h"
#include "code_8220.h"
#include "gte.h"

void FreeBasicClassList(BasicClassListNode **head)
{
    BasicClassListNode *node = *head;

    while (node != NULL) {
        BasicClassListNode *cur = node;
        node = node->next;
        func_80017CFC(cur);
    }
}

void BasicClass__NotifyParents(BasicClass *self, s32 arg1)
{
    BasicClassListNode *cursor = self->parentRefs;
    BasicClass *value;

    for (GetNextBasicClass(&value, &cursor); value != NULL; GetNextBasicClass(&value, &cursor)) {
        value->methods->slot38(value, self, arg1);
    }
}

void BasicClass__func_18350(void) {
}

void BasicClass__OnNotify(BasicClass *self, void *arg1, s32 arg2)
{
    if (arg2 == 1) {
        self->methods->removeChild(self, (BasicClass *)arg1);
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

void SetupPrimCode(void *arg0, void *arg1)
{
    u8 *a = (u8 *)arg0;

    if (*(s32 *)((u8 *)arg1 + 0x1C) != 0) {
        a[7] = a[7] | 0x2;
    } else {
        a[7] = a[7] & 0xFD;
    }

    if (D_8008E248 != 0) {
        a[7] = a[7] | 0x1;
    } else {
        a[7] = a[7] & 0xFE;
    }

    *((u8 *)arg1 + 0x14) = a[3];
    *((u8 *)arg1 + 0x15) = a[7];
}

s32 ProjectTriFace(void *arg0, u8 *prim, u16 idx0, u16 idx1, u16 idx2, void (*callback)(void *))
{
    *(void **)(prim + 0xa4) = (u8 *)(*(void **)(prim + 0xc)) + (s32)idx0 * 8;
    *(void **)(prim + 0xa8) = (u8 *)(*(void **)(prim + 0xc)) + (s32)idx1 * 8;
    *(void **)(prim + 0xac) = (u8 *)(*(void **)(prim + 0xc)) + (s32)idx2 * 8;

    gte_ldv3(*(void **)(prim + 0xa4), *(void **)(prim + 0xa8),
             *(void **)(prim + 0xac));

    if (TransformAndCullPoly(arg0, prim) != 0) {
        goto fail;
    }

    {
        u8 *p0 = *(u8 **)(prim + 0x88) + 0x14;
        u8 *p1 = *(u8 **)(prim + 0x8c) + 0x14;
        u8 *p2 = *(u8 **)(prim + 0x90) + 0x14;

        gte_stsz3(p0, p1, p2);
    }
    callback(arg0);
    func_8001A268(prim, 3);
    return 0;
fail:
    return 1;
}

s32 ProjectQuadFace(void *arg0, u8 *prim, u16 idx0, u16 idx1, u16 idx2, u16 idx3, void (*callback)(void *, s32))
{
    u8 *vtxSlot = prim + 0xa4;

    *(void **)(prim + 0xa4) = (u8 *)(*(void **)(prim + 0xc)) + (s32)idx0 * 8;
    *(void **)(prim + 0xa8) = (u8 *)(*(void **)(prim + 0xc)) + (s32)idx1 * 8;
    *(void **)(prim + 0xac) = (u8 *)(*(void **)(prim + 0xc)) + (s32)idx2 * 8;
    *(void **)(prim + 0xb0) = (u8 *)(*(void **)(prim + 0xc)) + (s32)idx3 * 8;

    gte_ldv3(*(void **)(vtxSlot + 0x0), *(void **)(vtxSlot + 0x4),
             *(void **)(vtxSlot + 0x8));

    if (TransformAndCullPoly(arg0, prim) != 0) {
        goto fail;
    }

    callback(arg0, 1);

    gte_ldv0(*(void **)(vtxSlot + 0xc));
    gte_rtps();

    {
        u8 *p0 = *(u8 **)(prim + 0x94) + 0x14;
        u8 *p1 = *(u8 **)(prim + 0x98) + 0x14;
        u8 *p2 = *(u8 **)(prim + 0x9c) + 0x14;
        u8 *p3 = *(u8 **)(prim + 0xa0) + 0x14;

        gte_stsz4(p0, p1, p2, p3);
    }

    callback(arg0, 0);

    gte_stsxy2(prim + 0x6c);

    func_8001A268(prim, 4);
    return 0;
fail:
    return 1;
}

/*
 * This unit's local view of the per-primitive GTE context that
 * TransformAndCullPoly works on (the `prim` its callers hand it). Only the fields
 * this function touches are typed; the callers still address the rest by
 * offset. The three SXY words are deliberately separate fields rather than
 * an array: retail stores them through three independently computed
 * addresses, which is gte_stsxy3()'s three-pointer form.
 */
typedef struct GteCullCtx {
    /* +0x000 */ u32 *otBase;        /* ordering table, 4-byte entries */
    /* +0x004 */ s32 otShift;        /* otz >> otShift indexes otBase */
    u8 pad008[0x014 - 0x008];
    /* +0x014 */ u8 unk14;           /* copied to the owner's byte at +0x3 */
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
} GteCullCtx;

/* The owner object: only its byte at +0x3 is written here. */
typedef struct GteCullOwner {
    u8 pad000[3];
    /* +0x003 */ u8 unk3;
} GteCullOwner;

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
s32 TransformAndCullPoly(void *arg0, void *arg1)
{
    GteCullOwner *owner = arg0;
    GteCullCtx *ctx = arg1;

    ctx->saturated = 0;
    gte_rtpt();
    owner->unk3 = ctx->unk14;
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

void StoreSxyPolyF3(void *dst)
{
    gte_stsxy3_f3(dst);
}

void StoreSxyPolyG3(void *dst)
{
    gte_stsxy3_g3(dst);
}

void StoreSxyPolyFT3(void *dst)
{
    gte_stsxy3_ft3(dst);
}

void StoreSxyPolyGT3(void *dst)
{
    gte_stsxy3_gt3(dst);
}

void StoreSxyPolyF4(void *dst, s32 flag)
{
    char *p = (char *)dst + 0x14;

    if (flag) {
        gte_stsxy3_f4(dst);
    } else {
        gte_stsxy2(p);
    }
}

void StoreSxyPolyG4(void *dst, s32 flag)
{
    char *p = (char *)dst + 0x20;

    if (flag) {
        gte_stsxy3_g4(dst);
    } else {
        gte_stsxy2(p);
    }
}
